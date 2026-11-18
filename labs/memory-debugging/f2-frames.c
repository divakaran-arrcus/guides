/* f2-frames.c — a three-deep call chain to look at stack frames.
 * main -> rx_packet -> parse_header -> checksum. Stop in checksum and walk
 * back up: each frame holds its saved rbp and the return address (saved rip).
 *
 *   make f2-frames.plain && gdb -x gdb/f2-frames.gdb ./f2-frames.plain
 */
#include "pktd.h"

static uint16_t checksum(const char *data, uint32_t len)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < len; i++)
        sum += (unsigned char)data[i];
    return (uint16_t)(sum + (sum >> 16));
}

static int parse_header(struct pktbuf *pb)
{
    uint16_t csum = checksum(pb->data, pb->len);
    return csum != 0;
}

static int rx_packet(uint32_t port)
{
    struct pktbuf pb = { .len = 4, .port = port, .ts = 0 };
    memcpy(pb.data, "BGP!", 4);
    return parse_header(&pb);
}

int main(void)
{
    int ok = rx_packet(179);
    printf("rx_packet: %s\n", ok ? "ok" : "bad");
    return 0;
}
