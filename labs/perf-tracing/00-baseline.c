/* 00-baseline.c — the healthy pipeline: one SPF, a short RIB update, a batched push. */
#include "pipeline.h"

int main(int argc, char **argv) {
  struct cfg c = { .lab = "00-baseline", .fixed = arg_fixed(argc, argv), .nodes = 1500, .changes = 64 };
  return pipeline_main(&c);
}
