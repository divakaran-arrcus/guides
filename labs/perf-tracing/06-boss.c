/* 06-boss.c — the boss level: four slowdowns at once.
 *   ./06-boss                      nothing fixed
 *   ./06-boss --fix=push,lock      fix some of them (any of: hot, lock, push, hold, all)
 * hot  = SPF per change (rung 1)       lock = walker holds rib_lock (rung 2)
 * push = stop-and-wait push (rung 3)   hold = backoff never resets (rung 5) */
#include "pipeline.h"

static int has(const char *list, const char *w) {
  if (!list) return 0;
  size_t n = strlen(w);
  for (const char *p = list; *p; ) {
    const char *e = strchr(p, ',');
    size_t len = e ? (size_t)(e - p) : strlen(p);
    if ((len == n && strncmp(p, w, n) == 0) || (len == 3 && strncmp(p, "all", 3) == 0)) return 1;
    if (!e) break;
    p = e + 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  const char *fix = NULL;
  for (int i = 1; i < argc; i++) if (strncmp(argv[i], "--fix=", 6) == 0) fix = argv[i] + 6;
  struct cfg c = { .lab = "06-boss", .fixed = fix != NULL, .nodes = 1500, .changes = 32,
                   .spf_per_change = !has(fix, "hot"),
                   .cli_walk = 1, .walk_holds_lock = !has(fix, "lock"),
                   .stop_and_wait = !has(fix, "push"),
                   .flaps = 2, .hold_reset = has(fix, "hold") };
  if (fix) printf("fixes applied: %s\n", fix);
  return pipeline_main(&c);
}
