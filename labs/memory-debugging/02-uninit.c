/* 02-uninit.c — a session's flags come from a stack variable that is only
 * written when a config word parses. With no options it is never written,
 * and the value is whatever the stack held before: it changes with the
 * build, the compiler and the caller.
 *
 *   make 02-uninit.plain   && valgrind --track-origins=yes ./02-uninit.plain
 *   make 02-uninit.msan    && ./02-uninit.msan
 *   make 02-uninit.pattern && ./02-uninit.pattern   (GCC fills it with 0xFE)
 */
#include "pktd.h"

#define SESS_F_PASSIVE 0x1u
#define SESS_F_MD5     0x2u

/* Parse one config word. Writes *out only on success. Kept out of line, as
 * it would be in its own file, so the optimizer can't see through it. */
__attribute__((noinline)) static int parse_opt(const char *word, uint32_t *out)
{
    if (strcmp(word, "passive") == 0) { *out = SESS_F_PASSIVE; return 0; }
    if (strcmp(word, "md5") == 0)     { *out = SESS_F_MD5;     return 0; }
    return -1;
}

static void session_apply_opts(struct session *s, int argc, char **argv)
{
    uint32_t flags;                 /* BUG: never initialised */

    for (int i = 1; i < argc; i++)
        parse_opt(argv[i], &flags); /* return value ignored, too */
    s->flags = flags;
}

int main(int argc, char **argv)
{
    static const struct peer p = { 0x0a000002, 179 };
    struct session *s = session_new(2, &p);

    session_apply_opts(s, argc, argv);   /* no options given: flags is junk */
    printf("session %u flags = %#x\n", s->id, s->flags);
    if (s->flags & SESS_F_PASSIVE)
        puts("passive: waiting for the peer to connect");
    else
        puts("active: connecting to the peer");
    session_put(s);
    return 0;
}
