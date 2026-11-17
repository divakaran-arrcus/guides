/* f3-chunks.c — what a malloc chunk looks like, before and after free.
 * Three 0x48-byte requests, each rounded up to a 0x50-byte chunk. Freeing
 * the middle one puts it in the tcache: its first word becomes the
 * safe-linked "next" pointer and its second word the tcache key.
 *
 *   make f3-chunks.plain && gdb -x gdb/f3-chunks.gdb ./f3-chunks.plain
 */
#include "pktd.h"

/* Empty marker functions so GDB can stop at a named point. */
__attribute__((noinline)) void before_free(void) { __asm__ volatile(""); }
__attribute__((noinline)) void after_free(void)  { __asm__ volatile(""); }

int main(void)
{
    char *a = malloc(0x48);
    char *p = malloc(0x48);
    char *c = malloc(0x48);
    memset(p, 'P', 0x48);

    before_free();
    free(p);
    after_free();

    printf("a=%p p=%p c=%p\n", (void *)a, (void *)p, (void *)c);
    free(a);
    free(c);
    return 0;
}
