# 10-rr.gdb — replay the recorded crash, then walk refcnt's writers backwards.
#   rr record ./10-refcount-uaf.plain
#   rr replay -o -q < gdb/10-rr.gdb
# Feed it on stdin: rr 5.7 runs "rr replay -x" files before it connects GDB
# to the replay, so a -x script finds no program to continue.
# The first reverse-continue stops on the SIGSEGV again (the fault is the
# last event); each one after that stops at the previous write to s->refcnt.
# Going backwards, "Old value" is the value after the write, "New value" the
# value before it; the HIT line prints them in that same order.
set pagination off
set width 0
continue
frame function table_dump
set $s = s
watch -l $s->refcnt
set $prev = $s->refcnt
commands
printf "HIT %d->%d handle_timeout=%d\n", $prev, $s->refcnt, $_caller_is("handle_timeout")
set $prev = $s->refcnt
end
reverse-continue
bt 2
reverse-continue
bt 2
reverse-continue
bt 2
reverse-continue
bt 2
reverse-continue
bt 2
quit
