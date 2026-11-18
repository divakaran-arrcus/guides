/* f1-layout.c — where things live in a process.
 * Prints the address of one object from each region, then copies this
 * process's own /proc/self/maps to stdout so you can find each address
 * in its mapping.
 *
 *   make f1-layout.plain && ./f1-layout.plain
 */
#include "pktd.h"

static const char banner[] = "pktd";        /* .rodata */
static int started;                          /* .bss    */

int main(void)
{
    struct peer peer = { 0x0a000001, 179 };  /* stack */
    struct session *s = session_new(1, &peer); /* heap  */
    started = 1;

    printf("text    main           %p\n", (void *)main);
    printf("rodata  banner         %p\n", (void *)banner);
    printf("bss     session_table  %p\n", (void *)session_table);
    printf("bss     started        %p\n", (void *)&started);
    printf("heap    session        %p\n", (void *)s);
    printf("stack   peer           %p\n", (void *)&peer);
    printf("libc    printf         %p\n", (void *)printf);
    printf("\n");

    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) {
        perror("/proc/self/maps");
        return 1;
    }
    char line[512];
    while (fgets(line, sizeof line, f))
        fputs(line, stdout);
    fclose(f);

    session_put(s);
    return 0;
}
