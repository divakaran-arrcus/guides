/* 04-stack-smash.c — the control thread copies a peer's hostname into a
 * 16-byte stack buffer with no length check. The copy runs over the stack
 * canary, the saved rbp and the saved return address. The canary check at
 * the end of the function catches it before the bad return happens.
 *
 *   make 04-stack-smash.plain && ./04-stack-smash.plain
 *   make 04-stack-smash.asan  && ./04-stack-smash.asan
 *   make core-04-stack-smash
 */
#include "pktd.h"

static int ctrl_set_hostname(struct session *s, const char *hostname)
{
    char host[16];

    strcpy(host, hostname);         /* BUG: no bound on the copy */
    snprintf(s->name, sizeof s->name, "%s", host);
    return (int)strlen(host);
}                                   /* canary checked here: __stack_chk_fail */

int main(void)
{
    static const struct peer p = { 0x0a000004, 179 };
    struct session *s = session_new(4, &p);
    /* 46 chars: well past host[16], the canary and the saved registers. */
    const char *from_peer = "edge-router-07.pop-ams.example.net.backup-link";

    int n = ctrl_set_hostname(s, from_peer);
    printf("hostname set (%d chars): %s\n", n, s->name);
    session_put(s);
    return 0;
}
