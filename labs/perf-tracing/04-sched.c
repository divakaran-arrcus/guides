/* 04-sched.c — rung 4: ready threads wait for a CPU.
 * The process runs on 2 CPUs with 8 I/O poller threads, and the pipeline threads were set to
 * nice 10 "to protect the pollers". Before: the pollers spin, so the pipeline threads sit in the
 * run queue behind them. --fixed: the pollers sleep until there is work (the nice stays). */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "04-sched", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .pollers = 8, .poll_busy = !fixed, .cpus = 2, .pipeline_nice = 10,
                   .schedstat = 1 };
  return pipeline_main(&c);
}
