/* 05-double-free.c — an error path frees a session but leaves it in
 * session_table. Shutdown later walks the table and frees it again.
 *
 *   make 05-double-free.plain && ./05-double-free.plain
 *   make 05-double-free.asan  && ./05-double-free.asan
 *   make core-05-double-free
 */
#include "pktd.h"

static int session_add(uint32_t id, const struct peer *p)
{
    struct session *s = session_new(id, p);

    session_table[id % MAX_SESSIONS] = s;
    if (p->port == 0) {             /* invalid peer: undo */
        free(s);                    /* BUG: the table still points at s */
        return -1;
    }
    return 0;
}

static void pktd_shutdown(void)
{
    for (int i = 0; i < MAX_SESSIONS; i++) {
        free(session_table[i]);     /* frees id 5 a second time */
        session_table[i] = NULL;
    }
}

int main(void)
{
    static const struct peer good = { 0x0a000003, 179 };
    static const struct peer bad  = { 0x0a000005, 0 };

    session_add(3, &good);
    if (session_add(5, &bad) < 0)
        fprintf(stderr, "session 5: invalid peer port, dropped\n");
    pktd_shutdown();
    puts("pktd: clean shutdown");
    return 0;
}
