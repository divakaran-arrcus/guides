/* 09-deadlock.c — the stats thread takes stats_lock, then table_lock: the
 * reverse of pktd's documented order (table_lock -> stats_lock).
 *
 *   inversion  control finishes table -> stats, a barrier, then stats takes
 *              stats -> table. Never hangs, but both lock edges are recorded,
 *              so TSan and Helgrind report the cycle.
 *   hang       both threads sleep while holding their first lock: a real
 *              deadlock. Use it with gcore.
 *
 *   ./09-deadlock.tsan inversion
 *   valgrind --tool=helgrind ./09-deadlock.plain inversion
 *   ./09-deadlock.plain hang &  sleep 2;  gcore -o cores/09 $!
 */
#include <unistd.h>
#include "pktd.h"

static pthread_barrier_t phase;
static int hang;

/* Control thread: add a session, then count it. table_lock -> stats_lock. */
static void *control_thread(void *arg)
{
    static const struct peer p = { 0x0a000001, 179 };
    (void)arg;
    pthread_mutex_lock(&table_lock);
    session_table[1] = session_new(1, &p);
    if (hang)
        usleep(100000);
    pthread_mutex_lock(&stats_lock);
    stats.pkts++;
    pthread_mutex_unlock(&stats_lock);
    pthread_mutex_unlock(&table_lock);
    if (!hang)
        pthread_barrier_wait(&phase);
    return NULL;
}

/* Stats thread: snapshot the counters, then count live sessions. */
static void *stats_thread(void *arg)
{
    int live = 0;
    (void)arg;
    if (!hang)
        pthread_barrier_wait(&phase);
    pthread_mutex_lock(&stats_lock);
    if (hang)
        usleep(100000);
    pthread_mutex_lock(&table_lock);    /* BUG: stats_lock -> table_lock */
    for (int i = 0; i < MAX_SESSIONS; i++)
        live += session_table[i] != NULL;
    printf("stats: pkts=%lu live=%d\n", (unsigned long)stats.pkts, live);
    pthread_mutex_unlock(&table_lock);
    pthread_mutex_unlock(&stats_lock);
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t ctl, st;

    hang = argc > 1 && strcmp(argv[1], "hang") == 0;
    pthread_barrier_init(&phase, NULL, 2);
    pthread_create(&ctl, NULL, control_thread, NULL);
    pthread_create(&st, NULL, stats_thread, NULL);
    pthread_join(ctl, NULL);
    pthread_join(st, NULL);
    puts("pktd: clean exit");
    return 0;
}
