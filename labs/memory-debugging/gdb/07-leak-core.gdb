# 07-leak-core.gdb — a core of the leaking process: sessions in the heap, none in the table.
#   gdb -q -batch -nx -x gdb/07-leak-core.gdb ./07-leak.plain cores/07-leak.exit.core
# "info proc mappings" on a core lists only file-backed mappings (NT_FILE), so there
# is no [heap] line. The heap is one of the core's own load segments: libc's debug
# info says where the brk heap starts, and the Python below picks the segment
# ("info files") that contains that address.
set pagination off
source pktd-gdb.py
echo --- the table: what pktd still references ---\n
pktd-sessions
echo --- where the brk heap starts (libc's malloc state) ---\n
p/x mp_.sbrk_base
echo --- the core segment that holds it ---\n
python
import re
base = int(gdb.parse_and_eval("(unsigned long) mp_.sbrk_base"))
for line in gdb.execute("info files", to_string=True).splitlines():
    m = re.match(r"\s*(0x[0-9a-f]+) - (0x[0-9a-f]+) is (load\w*)\s*$", line)
    if m and int(m.group(1), 16) <= base < int(m.group(2), 16):
        print(line.strip())
        gdb.set_convenience_variable("heap_lo", int(m.group(1), 16))
        gdb.set_convenience_variable("heap_hi", int(m.group(2), 16))
        break
end
echo --- every 4-byte word in it equal to SESSION_MAGIC ---\n
find /w $heap_lo, $heap_hi - 4, 0x5E55102B
echo --- the last hit, read as a session ---\n
p *(struct session *)$_
