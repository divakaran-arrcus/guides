# 10-watch.gdb — catch every write to session 7's refcnt, whichever thread makes it.
#   gdb -q -batch -nx -x gdb/10-watch.gdb ./10-refcount-uaf.plain
# -l watches the address, not the expression, so the watchpoint survives
# frames returning and threads switching. Each hit prints one summary line
# (old->new, and whether the caller is handle_timeout), then a short backtrace.
set pagination off
break control_ready
run
watch -l session_table[7]->refcnt
set $prev = session_table[7]->refcnt
commands
printf "HIT %d->%d handle_timeout=%d\n", $prev, session_table[7]->refcnt, $_caller_is("handle_timeout")
set $prev = session_table[7]->refcnt
bt 3
continue
end
continue
