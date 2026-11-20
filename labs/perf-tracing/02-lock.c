/* 02-lock.c — rung 2: the RIB update waits for rib_lock.
 * A "show rib" walker starts when compute begins and writes every route to a slow terminal.
 * Before: it holds rib_lock for the whole walk. --fixed: copy under the lock, format outside it. */
#include "pipeline.h"

int main(int argc, char **argv) {
  int fixed = arg_fixed(argc, argv);
  struct cfg c = { .lab = "02-lock", .fixed = fixed, .nodes = 1500, .changes = 64,
                   .cli_walk = 1, .walk_holds_lock = !fixed };
  return pipeline_main(&c);
}
