/* pktd.h — the shared model for every lab: a toy packet daemon, "pktd".
 *
 * pktd keeps peer sessions in a fixed table and counts packets. Four workers,
 * one control thread and one stats thread share it. Lock order is always
 * table_lock -> stats_lock.
 *
 * Everything is defined here (static inline, plus the few globals), so each
 * lab is a single .c file and a single compile line.
 *
 * Build with -DPKTD_REFTRACE (make <lab>.reftrace) to turn on the refcount
 * tracer: every get/put is logged to a per-id side ring, the final put poisons
 * the session instead of freeing it, and session_check() aborts with that
 * history the first time anyone touches a dead session.
 */
#ifndef PKTD_H
#define PKTD_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef PKTD_REFTRACE
#include <execinfo.h>
#include <unistd.h>
#endif

#define SESSION_MAGIC 0x5E55102Bu   /* live session */
#define SESSION_DEAD  0xDEADBEEFu   /* poisoned by the final put (PKTD_REFTRACE) */
#define MAX_SESSIONS  64

struct peer {
    uint32_t addr;                  /* IPv4, host byte order */
    uint16_t port;
};

struct session {                    /* 56 bytes */
    uint32_t magic;                 /* 0x00 */
    int refcnt;                     /* 0x04 */
    uint32_t id;                    /* 0x08 */
    uint32_t flags;                 /* 0x0c */
    uint64_t created_ns;            /* 0x10 */
    const struct peer *peer;        /* 0x18 */
    char name[24];                  /* 0x20 */
};

struct pktbuf {                     /* 56 bytes */
    uint32_t len;
    uint32_t port;
    uint64_t ts;
    char data[40];
};

_Static_assert(sizeof(struct session) == 56, "struct session is 56 bytes");
_Static_assert(sizeof(struct pktbuf) == sizeof(struct session), "pktbuf and session share a size class");
_Static_assert(offsetof(struct session, created_ns) == 0x10, "created_ns at 0x10");
_Static_assert(offsetof(struct session, peer) == 0x18, "peer pointer at 0x18");

/* Shared state. The locks are declared extern like a real header would; all
 * of it is defined right here because every lab is one translation unit. */
extern pthread_mutex_t table_lock, stats_lock;

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;
struct session *session_table[MAX_SESSIONS];    /* under table_lock */
struct { uint64_t pkts, drops; } stats;          /* under stats_lock */

static inline uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000u + (uint64_t)ts.tv_nsec;
}

#ifdef PKTD_REFTRACE
/* ---- refcount tracer: a side table, outside struct session ---- */
#define REF_RING 16

struct ref_event {
    char op;                        /* 'N' new, 'G' get, 'P' put */
    int refcnt_after;
    void *caller;
    pid_t tid;
};

struct ref_ring {
    unsigned head;                  /* total events; slot = head % REF_RING */
    struct ref_event ev[REF_RING];
};

static struct ref_ring rings[MAX_SESSIONS];

static inline void ref_record(uint32_t id, char op, int after, void *caller)
{
    struct ref_ring *r = &rings[id % MAX_SESSIONS];
    unsigned slot = __atomic_fetch_add(&r->head, 1, __ATOMIC_RELAXED) % REF_RING;
    r->ev[slot] = (struct ref_event){ op, after, caller, gettid() };
}

static inline void ref_dump(uint32_t id)
{
    struct ref_ring *r = &rings[id % MAX_SESSIONS];
    unsigned n = r->head < REF_RING ? r->head : REF_RING;
    for (unsigned i = r->head - n; i != r->head; i++) {
        struct ref_event *e = &r->ev[i % REF_RING];
        fprintf(stderr, "  #%-2u %c refcnt=%d tid=%d  ", i, e->op, e->refcnt_after, (int)e->tid);
        fflush(stderr);
        backtrace_symbols_fd(&e->caller, 1, 2);
    }
}
#define REF_RECORD(s, op, after) ref_record((s)->id, (op), (after), __builtin_return_address(0))
#else
#define REF_RECORD(s, op, after) ((void)0)
#endif

/* ---- session lifetime ---- */

/* Labs build at -O0, so these stay real calls: GDB sees their frames and the
 * tracer's __builtin_return_address(0) is the caller of get/put. */
static inline struct session *session_new(uint32_t id, const struct peer *p)
{
    struct session *s = malloc(sizeof *s);
    if (!s)
        return NULL;
    s->magic = SESSION_MAGIC;
    s->refcnt = 1;
    s->id = id;
    s->flags = 0;
    s->created_ns = now_ns();
    s->peer = p;
    snprintf(s->name, sizeof s->name, "sess-%u", id);
    REF_RECORD(s, 'N', 1);
    return s;
}

static inline void session_get(struct session *s)
{
    int after = __atomic_add_fetch(&s->refcnt, 1, __ATOMIC_RELAXED);
    REF_RECORD(s, 'G', after);
    (void)after;
}

static inline void session_put(struct session *s)
{
    int after = __atomic_sub_fetch(&s->refcnt, 1, __ATOMIC_ACQ_REL);
    REF_RECORD(s, 'P', after);
    assert(after >= 0);
    if (after != 0)
        return;
#ifdef PKTD_REFTRACE
    /* Poison, don't free: keep refcnt and id, smash created_ns/peer/name
     * (0x10..0x37), and leave the chunk quarantined forever. */
    s->magic = SESSION_DEAD;
    memset((char *)s + offsetof(struct session, created_ns), 0x5A,
           sizeof *s - offsetof(struct session, created_ns));
#else
    free(s);
#endif
}

#ifdef PKTD_REFTRACE
static inline void session_check(struct session *s)
{
    if (s->magic != SESSION_DEAD)
        return;
    fprintf(stderr, "session %u: use after final put\n", s->id);
    ref_dump(s->id);
    abort();
}
#else
static inline void session_check(struct session *s) { (void)s; }
#endif

/* Print every live entry. session_check() runs before any field is read. */
static inline void table_dump(void)
{
    pthread_mutex_lock(&table_lock);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        struct session *s = session_table[i];
        if (!s)
            continue;
        session_check(s);
        printf("[%2d] id=%u refcnt=%d name=%s peer=%u.%u.%u.%u:%u\n", i, s->id,
               s->refcnt, s->name, s->peer->addr >> 24, (s->peer->addr >> 16) & 0xff,
               (s->peer->addr >> 8) & 0xff, s->peer->addr & 0xff, s->peer->port);
    }
    pthread_mutex_unlock(&table_lock);
}

#endif /* PKTD_H */
