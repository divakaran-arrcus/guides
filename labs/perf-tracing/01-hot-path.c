/* 01-hot-path.c — rung 1: compute is slow because SPF runs once per link change.
 * Before: compute_routes() runs a full O(N^2) SPF for each of the 64 changes.
 * --fixed: apply all 64 changes, then run SPF once. */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "01-hot-path", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .spf_per_change = !fixed };
  return pipeline_main(&c);
}
