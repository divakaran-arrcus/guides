/* 10-refcount-uaf.c — boss level: a refcount bug whose crash lands far away.
 *
 * Worker 3's error path, handle_timeout(), drops a reference it never took.
 * Worker 2's own, correct put then takes session 7 to zero, and free() puts
 * the chunk in worker 2's tcache. Worker 2's next malloc is a struct pktbuf:
 * same 56-byte size class, same thread, so tcache hands back the same chunk.
 * Later the control thread's table_dump() reads session_table[7]->peer, which
 * is now packet bytes "IJKLMNOP" = 0x504f4e4d4c4b4a49. That address is
 * non-canonical, so the read is a SIGSEGV in code that did nothing wrong.
 *
 * Barriers put the six threads through the same phases on every run:
 *   0  control   creates sessions 1-8 (7 -> peers[2], refcnt 1); control_ready()
 *   1  worker 2  session_get(7)                       refcnt 1 -> 2
 *   2  worker 3  handle_timeout(7): a put, no get     refcnt 2 -> 1   (the bug)
 *   3  worker 2  session_put(7)                       refcnt 1 -> 0, free()
 *                malloc() + fill a pktbuf             gets the same chunk
 *   4  control   table_dump()                         SIGSEGV on session 7
 *
 *   ./10-refcount-uaf.plain              SIGSEGV in table_dump, every run
 *   make core-10-refcount-uaf            GDB writes cores/10-refcount-uaf.plain.core
 *   ./10-refcount-uaf.asan               heap-use-after-free, with the free's stack
 *   ./10-refcount-uaf.reftrace           session_check() prints the get/put history
 *
 * worker_main, handle_timeout and control_main are not static, so the
 * reftrace build (-rdynamic) can name them in its history.
 */
#include "pktd.h"

#define WORKERS 4
#define THREADS (WORKERS + 2)       /* + control + stats */

static pthread_barrier_t phase;
static const struct peer peers[4] = {
    { 0x0a000001, 179 }, { 0x0a000002, 179 }, { 0x0a000003, 179 }, { 0x0a000004, 179 },
};

/* A hook for debuggers: sessions 1-8 exist and no worker has touched them. */
__attribute__((noinline)) void control_ready(void) { __asm__ volatile("" ::: "memory"); }

static struct session *lookup(uint32_t id)
{
    pthread_mutex_lock(&table_lock);
    struct session *s = session_table[id];
    pthread_mutex_unlock(&table_lock);
    return s;
}

/* Error path: the peer timed out, so count a drop and let the session go.
 * BUG: this thread never called session_get(), so it drops a reference
 * that belongs to someone else. */
void handle_timeout(struct session *s)
{
    pthread_mutex_lock(&stats_lock);
    stats.drops++;
    pthread_mutex_unlock(&stats_lock);
    session_put(s);
}

void *worker_main(void *arg)
{
    int me = (int)(intptr_t)arg;    /* 1..4 */
    struct session *s = NULL;
    struct pktbuf *pb = NULL;

    pthread_barrier_wait(&phase);   /* 0: sessions exist */
    if (me == 2) {
        s = lookup(7);
        session_get(s);             /* 1: our reference, 1 -> 2 */
    }
    pthread_barrier_wait(&phase);
    if (me == 3)
        handle_timeout(lookup(7));  /* 2: 2 -> 1 */
    pthread_barrier_wait(&phase);
    if (me == 2) {
        session_put(s);             /* 3: our put, correct, but 1 -> 0: free() */
        pb = malloc(sizeof *pb);    /* the next packet: same thread, same bin */
        pb->len = sizeof pb->data;
        pb->port = 179;
        pb->ts = now_ns();
        memcpy(pb->data, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklm", sizeof pb->data);
    }
    pthread_barrier_wait(&phase);   /* 4: control dumps the table */
    pthread_barrier_wait(&phase);   /* done */
    free(pb);
    return NULL;
}

void *control_main(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&table_lock);
    for (uint32_t id = 1; id <= 8; id++)
        session_table[id] = session_new(id, &peers[(id - 1) % 4]);
    pthread_mutex_unlock(&table_lock);
    control_ready();
    for (int i = 0; i < 4; i++)     /* let the workers run phases 0-3 */
        pthread_barrier_wait(&phase);
    table_dump();                   /* 4: crashes on session 7 */
    pthread_barrier_wait(&phase);
    return NULL;
}

static void *stats_main(void *arg)
{
    (void)arg;
    for (int i = 0; i < 5; i++)
        pthread_barrier_wait(&phase);
    pthread_mutex_lock(&stats_lock);
    printf("stats: pkts=%lu drops=%lu\n", (unsigned long)stats.pkts, (unsigned long)stats.drops);
    pthread_mutex_unlock(&stats_lock);
    return NULL;
}

int main(void)
{
    pthread_t tid[THREADS];

    setvbuf(stdout, NULL, _IOLBF, 0);   /* each table line is out before the crash */
    pthread_barrier_init(&phase, NULL, THREADS);
    pthread_create(&tid[0], NULL, control_main, NULL);
    for (int i = 1; i <= WORKERS; i++)
        pthread_create(&tid[i], NULL, worker_main, (void *)(intptr_t)i);
    pthread_create(&tid[THREADS - 1], NULL, stats_main, NULL);
    for (int i = 0; i < THREADS; i++)
        pthread_join(tid[i], NULL);
    puts("pktd: clean exit");
    return 0;
}
