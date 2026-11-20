/* 03-syscalls.c — rung 3: push is slow because every route costs a write and an ack.
 * Before: stop-and-wait, one route per message. --fixed: 512 routes per message, one ack each. */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "03-syscalls", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .stop_and_wait = !fixed };
  return pipeline_main(&c);
}
