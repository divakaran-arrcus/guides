/* 07-leak.c — peers connect and disconnect. session_close() unlinks the
 * session from session_table but never drops the table's reference, so every
 * closed session leaks. The rx buffers are temporary and freed correctly.
 *
 *   make 07-leak.asan && ./07-leak.asan                  (LeakSanitizer)
 *   valgrind --leak-check=full ./07-leak.plain
 *   valgrind --tool=massif ./07-leak.plain; ms_print massif.out.*
 *   heaptrack ./07-leak.plain
 */
#include "pktd.h"

#define CHURN 2000                  /* connect/disconnect cycles */

static void session_open(uint32_t id, const struct peer *p)
{
    pthread_mutex_lock(&table_lock);
    session_table[id % MAX_SESSIONS] = session_new(id, p);
    pthread_mutex_unlock(&table_lock);
}

static void session_rx(uint32_t id)
{
    struct pktbuf *b = malloc(sizeof *b);   /* temporary: freed below */
    b->len = 64;
    b->port = 179;
    b->ts = now_ns();
    snprintf(b->data, sizeof b->data, "keepalive %u", id);
    stats.pkts++;
    free(b);
}

static void session_close(uint32_t id)
{
    pthread_mutex_lock(&table_lock);
    session_table[id % MAX_SESSIONS] = NULL;    /* BUG: no session_put() */
    pthread_mutex_unlock(&table_lock);
}

int main(void)
{
    static const struct peer p = { 0x0a000001, 179 };

    for (uint32_t id = 0; id < CHURN; id++) {
        session_open(id, &p);
        for (int k = 0; k < 4; k++)
            session_rx(id);
        session_close(id);
    }
    printf("pktd: %u sessions opened and closed, %lu pkts\n", CHURN,
           (unsigned long)stats.pkts);
    return 0;
}
