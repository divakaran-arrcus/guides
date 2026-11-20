/* 05-throttle.c — rung 5: nothing is slow on the CPU; the SPF throttle is waiting.
 * Six flaps ten minutes ago doubled the hold six times. Before: the backoff never resets.
 * --fixed: after 30 s of quiet the hold goes back to its initial value. */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "05-throttle", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .flaps = 6, .hold_reset = fixed };
  return pipeline_main(&c);
}
