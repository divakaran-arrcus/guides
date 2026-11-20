/* 04-sched.c — rung 4: a ready thread waits for a CPU.
 * The process runs on 2 CPUs with 8 I/O poller threads, and the SPF thread was set to nice 15
 * as "background work". Before: the pollers spin, so the SPF thread sits in the run queue behind
 * them. --fixed: the pollers sleep until there is work (the nice stays). */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "04-sched", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .pollers = 8, .poll_busy = !fixed, .cpus = 2, .spf_nice = 15,
                   .schedstat = 1 };
  return pipeline_main(&c);
}
