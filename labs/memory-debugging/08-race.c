/* 08-race.c — four workers count packets into stats.pkts without stats_lock.
 * "++" is load, add, store: two workers interleave and one update is lost.
 * Run with "atomic" to make the increment a single atomic read-modify-write.
 *
 *   ./08-race.plain                         (lost updates, no tool)
 *   make 08-race.tsan && ./08-race.tsan     (ThreadSanitizer)
 *   valgrind --tool=helgrind ./08-race.plain
 *   valgrind --tool=drd ./08-race.plain
 *   ./08-race.tsan atomic                   (fixed: no report, exact count)
 */
#include <stdatomic.h>
#include "pktd.h"

#define WORKERS 4
#define PKTS_PER_WORKER 1000000

static int use_atomic;

static void *worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < PKTS_PER_WORKER; i++) {
        if (use_atomic)
            atomic_fetch_add((_Atomic uint64_t *)&stats.pkts, 1);
        else
            stats.pkts++;           /* BUG: stats_lock not held */
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t tid[WORKERS];

    use_atomic = argc > 1 && strcmp(argv[1], "atomic") == 0;
    for (int i = 0; i < WORKERS; i++)
        pthread_create(&tid[i], NULL, worker, NULL);
    for (int i = 0; i < WORKERS; i++)
        pthread_join(tid[i], NULL);

    uint64_t expected = (uint64_t)WORKERS * PKTS_PER_WORKER;
    printf("expected=%lu actual=%lu lost=%lu\n", (unsigned long)expected,
           (unsigned long)stats.pkts, (unsigned long)(expected - stats.pkts));
    return 0;
}
