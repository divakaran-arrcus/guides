# pktd-gdb.py — GDB helper for pktd: the "pktd-sessions" command.
#   (gdb) source pktd-gdb.py
#   (gdb) pktd-sessions
# Walks session_table[] and prints one line per non-NULL slot. It checks the
# magic before trusting any other field, so a freed, poisoned or garbage
# entry is labelled rather than dereferenced blindly. Works live or on a core.
import gdb

SESSION_MAGIC = 0x5E55102B
SESSION_DEAD = 0xDEADBEEF


def _peer(s):
    p = s["peer"]
    if int(p) == 0:
        return "NULL"
    try:
        addr = int(p.dereference()["addr"])
        port = int(p.dereference()["port"])
    except gdb.MemoryError:
        return "%#x (unreadable)" % int(p)
    return "%d.%d.%d.%d:%d" % (addr >> 24, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF, port)


class PktdSessions(gdb.Command):
    """List pktd's session_table: slot, address, state, refcnt, id, name, peer."""

    def __init__(self):
        super().__init__("pktd-sessions", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        table = gdb.parse_and_eval("session_table")
        n = table.type.range()[1] + 1
        used = 0
        for i in range(n):
            s = table[i]
            if int(s) == 0:
                continue
            used += 1
            head = "[%2d] %#x" % (i, int(s))
            try:
                d = s.dereference()
                magic = int(d["magic"]) & 0xFFFFFFFF
                refcnt = int(d["refcnt"])
                sid = int(d["id"])
            except gdb.MemoryError:
                gdb.write("%s  UNREADABLE\n" % head)
                continue
            if magic == SESSION_MAGIC:
                name = d["name"].string(length=24, errors="replace").split("\0")[0]
                gdb.write("%s  live  refcnt=%d id=%u name=%s peer=%s\n"
                          % (head, refcnt, sid, name, _peer(d)))
            elif magic == SESSION_DEAD:
                gdb.write("%s  DEAD  refcnt=%d id=%u  (poisoned by final put)\n"
                          % (head, refcnt, sid))
            else:
                gdb.write("%s  BAD MAGIC %#010x refcnt=%d id=%u  (freed or corrupt?)\n"
                          % (head, magic, refcnt, sid))
        gdb.write("%d of %d slots in use\n" % (used, n))


PktdSessions()
