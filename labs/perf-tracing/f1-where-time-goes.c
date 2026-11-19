/* f1-where-time-goes.c — where a thread's wall time goes.
 *   spin   always on a CPU
 *   sleep  blocked in clock_nanosleep for 1 s (off-CPU)
 *   lock   blocked on a mutex another thread holds for 900 ms, then 100 ms of work
 *   q0-q2  three spinners pinned to one CPU: runnable, but mostly waiting in the run queue
 * Each thread reads its own schedstat (on-CPU ns, run-queue wait ns) before it exits;
 * whatever is left of its wall time was spent blocked (off-CPU). */
#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t out = PTHREAD_MUTEX_INITIALIZER;
static int qn;

static double now_s(clockid_t c) {
  struct timespec ts;
  clock_gettime(c, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void report(const char *name, double t_start) {
  double wall = now_s(CLOCK_MONOTONIC) - t_start;
  unsigned long long run = 0, wait = 0;
  FILE *f = fopen("/proc/thread-self/schedstat", "r");
  if (f) { if (fscanf(f, "%llu %llu", &run, &wait) != 2) run = wait = 0; fclose(f); }
  double off = wall * 1e3 - run / 1e6 - wait / 1e6;
  pthread_mutex_lock(&out);
  printf("%-6s wall %5.0f ms = on-cpu %5.0f + runq-wait %5.0f + off-cpu %5.0f\n", name,
         wall * 1e3, run / 1e6, wait / 1e6, off < 0 ? 0 : off);
  pthread_mutex_unlock(&out);
}

/* Burn this thread's own CPU time, not wall time: on a shared CPU this takes longer. */
static void burn_cpu(double s) {
  double end = now_s(CLOCK_THREAD_CPUTIME_ID) + s;
  while (now_s(CLOCK_THREAD_CPUTIME_ID) < end) ;
}

static void *t_spin(void *a) {
  (void)a;
  double t = now_s(CLOCK_MONOTONIC);
  burn_cpu(1.0);
  report("spin", t);
  return NULL;
}

static void *t_sleep(void *a) {
  (void)a;
  double t = now_s(CLOCK_MONOTONIC);
  struct timespec ts = { 1, 0 };
  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
  report("sleep", t);
  return NULL;
}

static void *t_holder(void *a) {
  (void)a;
  pthread_mutex_lock(&mu);
  usleep(900000);
  pthread_mutex_unlock(&mu);
  return NULL;
}

static void *t_lock(void *a) {
  (void)a;
  usleep(1000); /* let the holder take the lock first */
  double t = now_s(CLOCK_MONOTONIC);
  pthread_mutex_lock(&mu);
  pthread_mutex_unlock(&mu);
  burn_cpu(0.1);
  report("lock", t);
  return NULL;
}

static void *t_queue(void *a) {
  cpu_set_t one;
  CPU_ZERO(&one);
  CPU_SET((int)(long)a, &one);
  pthread_setaffinity_np(pthread_self(), sizeof one, &one);
  double t = now_s(CLOCK_MONOTONIC);
  burn_cpu(0.333);
  char name[8];
  pthread_mutex_lock(&out);
  snprintf(name, sizeof name, "q%d", qn++);
  pthread_mutex_unlock(&out);
  report(name, t);
  return NULL;
}

int main(void) {
  cpu_set_t all;
  long cpu = 0;
  sched_getaffinity(0, sizeof all, &all);
  for (int c = CPU_SETSIZE - 1; c >= 0; c--) if (CPU_ISSET(c, &all)) { cpu = c; break; }
  pthread_t t[7];
  pthread_create(&t[0], NULL, t_spin, NULL);
  pthread_create(&t[1], NULL, t_sleep, NULL);
  pthread_create(&t[2], NULL, t_holder, NULL);
  pthread_create(&t[3], NULL, t_lock, NULL);
  for (int i = 0; i < 3; i++) pthread_create(&t[4 + i], NULL, t_queue, (void *)cpu);
  for (int i = 0; i < 7; i++) pthread_join(t[i], NULL);
  return 0;
}
