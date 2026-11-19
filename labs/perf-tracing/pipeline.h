/* pipeline.h — pktd's route pipeline (fictional, illustrative; not taken from any real daemon).
 *
 * One convergence event flows through four stages, each on its own thread:
 *
 *   control (pktd-ctl)  injects a link change at T0
 *   hold    (pktd-spf)  SPF throttle: wait before computing
 *   compute (pktd-spf)  shortest-path first over the topology
 *   rib     (pktd-spf)  write every route into the shared RIB under rib_lock
 *   push    (pktd-push) send the routes to the FIB thread (pktd-fib) and wait for acks
 *
 * "Converged" is the moment the last FIB ack arrives. Every stage stamps CLOCK_MONOTONIC and
 * fires a USDT probe, and pipeline_main() prints the timeline and a per-stage budget at exit.
 *
 * Each lab sets a struct cfg that plants one slowdown (or several, for the boss level);
 * --fixed (or --fix=... for the boss) selects the fix. Build: see the Makefile.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/sdt.h>
#include <time.h>
#include <unistd.h>

#define NOINLINE __attribute__((noinline))

/* ---------- configuration: what each lab plants ---------- */
struct cfg {
  const char *lab;
  int fixed;           /* labs 01-05: 1 = the fixed variant */
  int nodes;           /* topology size */
  int changes;         /* link changes in the event */
  int spf_per_change;  /* bug 01: run a full SPF for every change instead of once */
  int cli_walk;        /* a "show rib" walker starts when compute begins */
  int walk_holds_lock; /* bug 02: the walker formats and writes while holding rib_lock */
  int stop_and_wait;   /* bug 03: push one route per message, wait for one ack each */
  int pollers;         /* number of I/O poller threads */
  int poll_busy;       /* bug 04: pollers spin instead of sleeping */
  int cpus;            /* pin the process to this many CPUs (0 = no pinning) */
  int flaps;           /* bug 05: earlier flaps already in the throttle's history */
  int hold_reset;      /* fix 05: reset the backoff after a quiet period */
  int schedstat;       /* print per-thread schedstat at exit */
  int pipeline_nice;   /* bug 04: nice value of the pipeline threads (spf, push, fib) */
};

#define PREFIXES_PER_NODE 16
#define BATCH 512
#define HOLD_INIT_MS 50
#define HOLD_MAX_MS 10000
#define QUIET_RESET_S 30

/* ---------- the timeline ---------- */
enum stage { ST_HOLD, ST_COMPUTE, ST_RIB, ST_PUSH, ST_N };
static const char *stage_name[ST_N] = { "hold", "compute", "rib", "push" };
static uint64_t t0_ns, st_begin[ST_N], st_end[ST_N];

static uint64_t now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}
static void tl_begin(enum stage s) { st_begin[s] = now_ns(); DTRACE_PROBE2(pktd, stage, s, 1); }
static void tl_end(enum stage s)   { st_end[s] = now_ns();   DTRACE_PROBE2(pktd, stage, s, 0); }
static double ms(uint64_t a, uint64_t b) { return (double)(b - a) / 1e6; }

/* ---------- topology, SPF, RIB ---------- */
struct route { uint32_t prefix; uint32_t nexthop; uint32_t metric; uint32_t flags; };

static struct cfg C;
static int N, R;
static uint16_t *w;                 /* N x N link weights, 0 = no link */
static uint32_t *dist, *firsthop;
static struct route *rib;
static pthread_mutex_t rib_lock = PTHREAD_MUTEX_INITIALIZER;

static uint32_t rnd_state = 12345;
static uint32_t rnd(void) { rnd_state = rnd_state * 1103515245u + 12345u; return rnd_state >> 8; }

static void topo_build(void) {
  w = calloc((size_t)N * N, sizeof *w);
  dist = malloc(N * sizeof *dist);
  firsthop = malloc(N * sizeof *firsthop);
  rib = calloc(R, sizeof *rib);
  for (int u = 0; u < N; u++)
    for (int k = 0; k < 4; k++) {
      int v = (int)(rnd() % N);
      if (v == u) continue;
      w[u * N + v] = w[v * N + u] = (uint16_t)(1 + rnd() % 20);
    }
  for (int i = 0; i < R; i++) rib[i].prefix = 0x0a000000u | (uint32_t)i << 8;
}

/* Lock wrappers live in the binary, so a uprobe on them traces only this program. */
NOINLINE void rib_lock_acquire(void) { pthread_mutex_lock(&rib_lock); }
NOINLINE void rib_lock_release(void) { pthread_mutex_unlock(&rib_lock); }

NOINLINE static int spf_extract_min(const uint8_t *done) {
  int best = -1;
  uint32_t bd = UINT32_MAX;
  for (int v = 0; v < N; v++)
    if (!done[v] && dist[v] < bd) { bd = dist[v]; best = v; }
  return best;
}

NOINLINE static void spf_relax(int u, uint32_t *parent) {
  const uint16_t *row = &w[(size_t)u * N];
  for (int v = 0; v < N; v++)
    if (row[v] && dist[u] + row[v] < dist[v]) { dist[v] = dist[u] + row[v]; parent[v] = (uint32_t)u; }
}

/* O(N^2) Dijkstra from node 0: fine once, expensive when repeated. */
NOINLINE static void spf_run(void) {
  uint8_t *done = calloc(N, 1);
  uint32_t *parent = malloc(N * sizeof *parent);
  for (int v = 0; v < N; v++) { dist[v] = UINT32_MAX; parent[v] = UINT32_MAX; }
  dist[0] = 0;
  for (int i = 0; i < N; i++) {
    int u = spf_extract_min(done);
    if (u < 0) break;
    done[u] = 1;
    firsthop[u] = (u == 0) ? 0 : (parent[u] == 0 ? (uint32_t)u : firsthop[parent[u]]);
    spf_relax(u, parent);
  }
  free(done);
  free(parent);
}

static int changed_u[256], changed_v[256];

NOINLINE static void apply_change(int i) {
  int u = changed_u[i], v = changed_v[i];
  w[u * N + v] = w[v * N + u] = (uint16_t)(w[u * N + v] + 7);
}

/* Bug 01 lives here: one SPF per change instead of one SPF for the whole event. */
NOINLINE static void compute_routes(void) {
  for (int i = 0; i < C.changes; i++) {
    apply_change(i);
    if (C.spf_per_change) spf_run();
  }
  if (!C.spf_per_change) spf_run();
}

NOINLINE static void rib_update(void) {
  rib_lock_acquire();
  for (int i = 0; i < R; i++) {
    int node = i / PREFIXES_PER_NODE;
    rib[i].nexthop = firsthop[node];
    rib[i].metric = dist[node];
  }
  rib_lock_release();
}

/* Lab 04: someone lowered the pipeline threads' priority "to protect the I/O pollers".
 * On Linux, setpriority() on a thread id changes just that thread. Raising nice needs no privilege. */
static void set_nice(void) {
  if (C.pipeline_nice) setpriority(PRIO_PROCESS, (id_t)gettid(), C.pipeline_nice);
}

/* ---------- per-thread schedstat (lab 04) ---------- */
static char tnames[64][16];
static unsigned long long t_run[64], t_wait[64];
static int ntids;
static pthread_mutex_t track_mu = PTHREAD_MUTEX_INITIALIZER;

/* Each thread reads its own schedstat just before it exits: on-CPU ns, run-queue wait ns. */
static void schedstat_self(const char *name) {
  if (!C.schedstat) return;
  unsigned long long run = 0, wait = 0;
  FILE *f = fopen("/proc/thread-self/schedstat", "r");
  if (f) { if (fscanf(f, "%llu %llu", &run, &wait) != 2) run = wait = 0; fclose(f); }
  pthread_mutex_lock(&track_mu);
  if (ntids < 64) {
    snprintf(tnames[ntids], sizeof tnames[0], "%s", name);
    t_run[ntids] = run;
    t_wait[ntids] = wait;
    ntids++;
  }
  pthread_mutex_unlock(&track_mu);
}

/* ---------- the "show rib" walker and its slow terminal (lab 02) ---------- */
static int term_fd[2];
static pthread_mutex_t walk_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t walk_cv = PTHREAD_COND_INITIALIZER;
static int walk_go;

NOINLINE static void cli_format_and_write(const struct route *r, int n) {
  char line[96];
  for (int i = 0; i < n; i++) {
    int len = snprintf(line, sizeof line, "%u.%u.%u.0/24 via node %u metric %u\n",
                       r[i].prefix >> 24, (r[i].prefix >> 16) & 255, (r[i].prefix >> 8) & 255,
                       r[i].nexthop, r[i].metric);
    if (write(term_fd[1], line, (size_t)len) < 0) return;
  }
}

/* Bug 02 lives here: holding rib_lock while writing to a slow reader. */
NOINLINE static void cli_show_rib(void) {
  if (C.walk_holds_lock) {
    rib_lock_acquire();
    cli_format_and_write(rib, R);
    rib_lock_release();
  } else {
    struct route *snap = malloc(R * sizeof *snap);
    rib_lock_acquire();
    memcpy(snap, rib, R * sizeof *snap);
    rib_lock_release();
    cli_format_and_write(snap, R);
    free(snap);
  }
}

static void *cli_main(void *arg) {
  (void)arg;
  pthread_setname_np(pthread_self(), "pktd-cli");
  pthread_mutex_lock(&walk_mu);
  while (!walk_go) pthread_cond_wait(&walk_cv, &walk_mu);
  pthread_mutex_unlock(&walk_mu);
  cli_show_rib();
  close(term_fd[1]);
  return NULL;
}

/* A remote terminal that drains 4 KiB every 1.5 ms. */
static void *term_main(void *arg) {
  (void)arg;
  pthread_setname_np(pthread_self(), "pktd-term");
  char buf[4096];
  while (read(term_fd[0], buf, sizeof buf) > 0) usleep(1500);
  return NULL;
}

/* ---------- the SPF throttle (lab 05) ---------- */
static uint64_t thr_last_ns;
static int thr_hold_ms = HOLD_INIT_MS;

static void throttle_history(void) {
  /* The link flapped C.flaps times ten minutes ago; each flap doubled the hold. */
  for (int i = 0; i < C.flaps && thr_hold_ms < HOLD_MAX_MS; i++) thr_hold_ms *= 2;
  thr_last_ns = now_ns() - 600ull * 1000000000ull;
}

/* Bug 05 lives here: the backoff never resets after the network goes quiet. */
NOINLINE static void throttle_wait(void) {
  uint64_t t = now_ns();
  if (C.hold_reset && t - thr_last_ns > QUIET_RESET_S * 1000000000ull) thr_hold_ms = HOLD_INIT_MS;
  thr_last_ns = t;
  int hold = thr_hold_ms;
  DTRACE_PROBE1(pktd, hold, hold);
  struct timespec ts = { hold / 1000, (long)(hold % 1000) * 1000000L };
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  thr_hold_ms = hold * 2 > HOLD_MAX_MS ? HOLD_MAX_MS : hold * 2;
}

/* ---------- I/O pollers (lab 04) ---------- */
static atomic_int stop_all;
static atomic_int io_ready;
static pthread_mutex_t io_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t io_cv = PTHREAD_COND_INITIALIZER;

/* Bug 04 lives here: spinning on a flag instead of sleeping until there is work. */
NOINLINE static void io_poll_once(void) {
  if (C.poll_busy) {
    for (int i = 0; i < 1000 && !atomic_load(&io_ready); i++) __asm__ volatile("pause");
  } else {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 10000000;
    if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }
    pthread_mutex_lock(&io_mu);
    if (!atomic_load(&io_ready) && !atomic_load(&stop_all)) pthread_cond_timedwait(&io_cv, &io_mu, &ts);
    pthread_mutex_unlock(&io_mu);
  }
}

static void *poller_main(void *arg) {
  char name[16];
  snprintf(name, sizeof name, "pktd-io%ld", (long)(intptr_t)arg);
  pthread_setname_np(pthread_self(), name);
  while (!atomic_load(&stop_all)) io_poll_once();
  schedstat_self(name);
  return NULL;
}

/* ---------- push and FIB ---------- */
static int fib_fd[2];
static struct route *fib;

/* Bug 03 lives here: batch == 1 means one message and one ack per route. */
NOINLINE static void push_routes(void) {
  int batch = C.stop_and_wait ? 1 : BATCH;
  uint32_t ack;
  for (int i = 0; i < R; i += batch) {
    int n = R - i < batch ? R - i : batch;
    if (write(fib_fd[0], &rib[i], (size_t)n * sizeof *rib) < 0) { perror("push write"); exit(1); }
    if (read(fib_fd[0], &ack, sizeof ack) != sizeof ack) { perror("push ack"); exit(1); }
  }
}

static void *fib_main(void *arg) {
  (void)arg;
  pthread_setname_np(pthread_self(), "pktd-fib");
  set_nice();
  static struct route buf[BATCH];
  int have = 0;
  for (;;) {
    ssize_t got = read(fib_fd[1], buf, sizeof buf);
    if (got <= 0) break;
    int n = (int)(got / (ssize_t)sizeof *buf);
    memcpy(&fib[have], buf, (size_t)n * sizeof *buf);
    have += n;
    uint32_t ack = (uint32_t)have;
    if (write(fib_fd[1], &ack, sizeof ack) != sizeof ack) break;
  }
  schedstat_self("pktd-fib");
  return NULL;
}

/* ---------- threads that carry the event ---------- */
static pthread_mutex_t ev_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ev_cv = PTHREAD_COND_INITIALIZER;
static int ev_fired, rib_done;
static void *spf_main(void *arg) {
  (void)arg;
  pthread_setname_np(pthread_self(), "pktd-spf");
  set_nice();
  pthread_mutex_lock(&ev_mu);
  while (!ev_fired) pthread_cond_wait(&ev_cv, &ev_mu);
  pthread_mutex_unlock(&ev_mu);

  tl_begin(ST_HOLD);
  throttle_wait();
  tl_end(ST_HOLD);

  tl_begin(ST_COMPUTE);
  if (C.cli_walk) {
    pthread_mutex_lock(&walk_mu);
    walk_go = 1;
    pthread_cond_signal(&walk_cv);
    pthread_mutex_unlock(&walk_mu);
  }
  compute_routes();
  tl_end(ST_COMPUTE);

  tl_begin(ST_RIB);
  rib_update();
  tl_end(ST_RIB);

  pthread_mutex_lock(&ev_mu);
  rib_done = 1;
  pthread_cond_broadcast(&ev_cv);
  pthread_mutex_unlock(&ev_mu);
  schedstat_self("pktd-spf");
  return NULL;
}

static void *push_main(void *arg) {
  (void)arg;
  pthread_setname_np(pthread_self(), "pktd-push");
  set_nice();
  pthread_mutex_lock(&ev_mu);
  while (!rib_done) pthread_cond_wait(&ev_cv, &ev_mu);
  pthread_mutex_unlock(&ev_mu);
  tl_begin(ST_PUSH);
  push_routes();
  tl_end(ST_PUSH);
  schedstat_self("pktd-push");
  return NULL;
}

static void pin_cpus(int n) {
  cpu_set_t all, pick;
  CPU_ZERO(&pick);
  if (sched_getaffinity(0, sizeof all, &all) != 0) return;
  for (int c = CPU_SETSIZE - 1, got = 0; c >= 0 && got < n; c--)
    if (CPU_ISSET(c, &all)) { CPU_SET(c, &pick); got++; }
  sched_setaffinity(0, sizeof pick, &pick);
}

static void print_schedstat(void) {
  printf("\nschedstat  thread       on-cpu ms  runq-wait ms\n");
  for (int i = 0; i < ntids; i++)
    printf("schedstat  %-12s %9.1f %13.1f\n", tnames[i], t_run[i] / 1e6, t_wait[i] / 1e6);
}

static void print_timeline(void) {
  uint64_t conv = st_end[ST_PUSH];
  double total = ms(t0_ns, conv);
  printf("pktd route pipeline  lab=%s variant=%s  nodes=%d routes=%d changes=%d\n",
         C.lab, C.fixed ? "fixed" : "before", N, R, C.changes);
  printf("T+%10.3f ms  event      link change injected\n", 0.0);
  for (int s = 0; s < ST_N; s++) {
    printf("T+%10.3f ms  %-9s  begin\n", ms(t0_ns, st_begin[s]), stage_name[s]);
    printf("T+%10.3f ms  %-9s  end\n", ms(t0_ns, st_end[s]), stage_name[s]);
  }
  double handoff = ms(t0_ns, st_begin[ST_HOLD]) + ms(st_end[ST_HOLD], st_begin[ST_COMPUTE])
                 + ms(st_end[ST_COMPUTE], st_begin[ST_RIB]) + ms(st_end[ST_RIB], st_begin[ST_PUSH]);
  printf("\nbudget  stage      ms        share\n");
  for (int s = 0; s < ST_N; s++) {
    double d = ms(st_begin[s], st_end[s]);
    printf("budget  %-9s %9.3f  %5.1f%%\n", stage_name[s], d, 100.0 * d / total);
  }
  printf("budget  %-9s %9.3f  %5.1f%%\n", "handoff", handoff, 100.0 * handoff / total);
  printf("converged_ms=%.3f\n", total);
}

/* A tracer in a container knows this process only by its host PID, so it cannot read our
 * /proc/<pid>/maps to symbolize user stacks. PKTD_MAPS=<file> saves a copy for symbolize.py. */
static void save_maps(void) {
  const char *path = getenv("PKTD_MAPS");
  if (!path) return;
  FILE *in = fopen("/proc/self/maps", "r"), *out = fopen(path, "w");
  char line[512];
  while (in && out && fgets(line, sizeof line, in)) fputs(line, out);
  if (in) fclose(in);
  if (out) fclose(out);
}

/* Run one convergence event with the given configuration. */
static int pipeline_main(const struct cfg *cfg) {
  C = *cfg;
  N = C.nodes;
  R = N * PREFIXES_PER_NODE;
  if (C.cpus) pin_cpus(C.cpus);
  pthread_setname_np(pthread_self(), "pktd-ctl");
  topo_build();
  for (int i = 0; i < C.changes; i++) {
    changed_u[i] = (int)(rnd() % N);
    changed_v[i] = (changed_u[i] + 1 + (int)(rnd() % (N - 1))) % N;
  }
  spf_run(); /* initial convergence, not measured */
  throttle_history();
  fib = calloc(R, sizeof *fib);
  if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fib_fd) != 0) { perror("socketpair"); return 1; }
  int sz = 1 << 20;
  setsockopt(fib_fd[0], SOL_SOCKET, SO_SNDBUF, &sz, sizeof sz);

  pthread_t spf, push, fibt, cli, term, poll[32];
  pthread_create(&fibt, NULL, fib_main, NULL);
  pthread_create(&spf, NULL, spf_main, NULL);
  pthread_create(&push, NULL, push_main, NULL);
  if (C.cli_walk) {
    if (pipe(term_fd) != 0) { perror("pipe"); return 1; }
    pthread_create(&cli, NULL, cli_main, NULL);
    pthread_create(&term, NULL, term_main, NULL);
  }
  for (int i = 0; i < C.pollers && i < 32; i++) pthread_create(&poll[i], NULL, poller_main, (void *)(intptr_t)i);
  usleep(100000); /* let every thread reach its wait */

  pthread_mutex_lock(&ev_mu);
  t0_ns = now_ns();
  DTRACE_PROBE(pktd, event);
  ev_fired = 1;
  pthread_cond_broadcast(&ev_cv);
  pthread_mutex_unlock(&ev_mu);

  pthread_join(spf, NULL);
  pthread_join(push, NULL);
  DTRACE_PROBE(pktd, converged);
  atomic_store(&stop_all, 1);
  pthread_mutex_lock(&io_mu);
  pthread_cond_broadcast(&io_cv);
  pthread_mutex_unlock(&io_mu);
  for (int i = 0; i < C.pollers && i < 32; i++) pthread_join(poll[i], NULL);
  shutdown(fib_fd[0], SHUT_WR);
  pthread_join(fibt, NULL);
  if (C.cli_walk) { pthread_join(cli, NULL); pthread_join(term, NULL); }
  print_timeline();
  if (C.schedstat) print_schedstat();
  save_maps();
  return 0;
}

__attribute__((unused)) static int arg_fixed(int argc, char **argv) {
  for (int i = 1; i < argc; i++) if (strcmp(argv[i], "--fixed") == 0) return 1;
  return 0;
}
