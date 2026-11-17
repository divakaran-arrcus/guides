/* 01-null.c — three ways pktd dereferences NULL. Pick one with argv[1]:
 *   plain  session_lookup() misses and the caller uses the NULL it got back
 *   field  same miss, but the code reads s->peer->addr: the fault is at 0x18
 *   fnptr  the control thread dispatches an opcode with no handler: pc = 0
 *
 *   make core-01-null ARGS=field
 *   gdb ./01-null.plain cores/01-null.plain-field.core
 */
#include "pktd.h"

typedef int (*ctrl_fn)(struct session *s);

static int ctrl_show(struct session *s) { return printf("%s\n", s->name); }

/* Opcode table. CTRL_CLEAR (1) was never wired up: its slot is NULL. */
static ctrl_fn ctrl_ops[] = { ctrl_show, NULL };

/* Non-static so GDB's "info symbol" names it from the return address. */
int ctrl_dispatch(unsigned op, struct session *s)
{
    return ctrl_ops[op](s);         /* BUG: no NULL check on the slot */
}

static struct session *session_lookup(uint32_t id)
{
    return session_table[id % MAX_SESSIONS];   /* NULL if never added */
}

static uint32_t session_id(const struct session *s)
{
    return s->id;                   /* plain: faults at address 0x8 */
}

static void rx_plain(uint32_t id)
{
    struct session *s = session_lookup(id);
    printf("rx for session %u\n", session_id(s));   /* BUG: s unchecked */
}

static void rx_field(uint32_t id)
{
    struct session *s = session_lookup(id);
    uint32_t addr = s->peer->addr;  /* BUG: loads s->peer from 0x0 + 0x18 */
    printf("peer %#x\n", addr);
}

int main(int argc, char **argv)
{
    static const struct peer p = { 0x0a000001, 179 };
    session_table[1] = session_new(1, &p);
    const char *mode = argc > 1 ? argv[1] : "plain";

    if (strcmp(mode, "plain") == 0)
        rx_plain(7);
    else if (strcmp(mode, "field") == 0)
        rx_field(7);
    else if (strcmp(mode, "fnptr") == 0)
        ctrl_dispatch(1, session_table[1]);
    else
        fprintf(stderr, "usage: %s plain|field|fnptr\n", argv[0]);
    return 1;
}
