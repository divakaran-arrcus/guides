/* 06-uaf.c — a policy drop frees an rx_job, but the rx path keeps using it.
 *
 * struct rx_job is 40 bytes: a 0x30 chunk, a size class nothing else in this
 * program allocates, and only one job is ever allocated. So its tcache bin is
 * empty when rx_policy() frees it, and glibc's safe-linked 'next' pointer,
 * (pos >> 12) ^ NULL, lands on job->buf. The plain build then loads
 * buf->len from an unmapped address (job >> 12) and faults every time.
 *
 *   make 06-uaf.asan && ./06-uaf.asan
 *   valgrind ./06-uaf.plain
 *   make core-06-uaf
 */
#include "pktd.h"

struct rx_job {                     /* 40 bytes */
    struct pktbuf *buf;             /* 0x00  tcache 'next' after free */
    uint64_t enq_ns;                /* 0x08  tcache 'key' after free */
    uint64_t seq;                   /* 0x10 */
    uint32_t port;                  /* 0x18 */
    uint32_t prio;                  /* 0x1c */
    uint64_t flags;                 /* 0x20 */
};
_Static_assert(sizeof(struct rx_job) == 40, "rx_job is 40 bytes (0x30 chunk)");

static struct rx_job *rx_enqueue(struct pktbuf *b, uint64_t seq)
{
    struct rx_job *job = malloc(sizeof *job);
    job->buf = b;
    job->enq_ns = now_ns();
    job->seq = seq;
    job->port = b->port;
    job->prio = 0;
    job->flags = 0;
    return job;
}

/* Policy says drop: release the job. The caller is not told. */
static int rx_policy(struct rx_job *job)
{
    if (job->port == 179 && job->buf->len > 4096) {
        free(job);
        return -1;
    }
    return 0;
}

static uint32_t rx_process(struct rx_job *job)
{
    return job->buf->len;           /* BUG: job may already be freed */
}

int main(void)
{
    struct pktbuf *b = malloc(sizeof *b);
    b->len = 9000;                  /* oversized BGP frame: policy drops it */
    b->port = 179;
    b->ts = now_ns();

    struct rx_job *job = rx_enqueue(b, 1);
    if (rx_policy(job) < 0)
        fprintf(stderr, "rx: job 1 dropped by policy\n");
    printf("rx: job 1, %u bytes\n", rx_process(job));   /* BUG: ignores the drop */
    free(b);
    return 0;
}
