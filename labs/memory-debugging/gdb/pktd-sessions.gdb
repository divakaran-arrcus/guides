# pktd-sessions.gdb — stop pktd once its session table is built, then list it.
#   gdb -q -batch -nx -x gdb/pktd-sessions.gdb ./10-refcount-uaf.plain
# control_ready() runs just after the control thread creates sessions 1-8,
# before any worker touches them, so every slot should read "live refcnt=1".
# On a core, skip the break: "source pktd-gdb.py" then "pktd-sessions".
set pagination off
source pktd-gdb.py
break control_ready
run
pktd-sessions
kill
quit
