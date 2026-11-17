# pktd-sessions.gdb — run a pktd lab to its fault, then list session_table.
#   gdb -q -batch -nx -x gdb/pktd-sessions.gdb ./10-refcount-uaf.plain
set pagination off
source pktd-gdb.py
run
bt
pktd-sessions
