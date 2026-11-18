/* 03-heap-overflow.c — an off-by-one strcpy into a 72-byte heap buffer.
 * a and b are both malloc(0x48), so both live in 0x50-byte chunks, back to
 * back. The 72-char description fits a exactly; its terminating NUL lands at
 * a[0x48], the low byte of b's chunk size field: 0x51 becomes 0x00. Nothing
 * notices until free(b).
 *
 *   make 03-heap-overflow.asan && ./03-heap-overflow.asan
 *   make 03-heap-overflow.plain && ./03-heap-overflow.plain
 *   make core-03-heap-overflow
 */
#include "pktd.h"

#define DESC_MAX 0x48               /* 72 bytes "including" the NUL. It isn't. */

int main(void)
{
    /* The peer's description, 72 chars, straight from the config file. */
    static const char cfg_desc[] =
        "transit-peer AS64500 primary uplink to the provider edge, rack 7 slot 12";
    char *a = malloc(DESC_MAX);     /* description buffer */
    char *b = malloc(DESC_MAX);     /* the next session's scratch buffer */

    _Static_assert(sizeof cfg_desc - 1 == DESC_MAX, "description is 72 chars");
    strcpy(a, cfg_desc);            /* BUG: writes 73 bytes into 72 */
    printf("desc: %s\n", a);

    free(b);                        /* glibc reads b's size field: 0 */
    free(a);
    return 0;
}
