# 10-watch.gdb — catch every write to session 7's refcnt, whichever thread makes it.
#   gdb -q -batch -nx -x gdb/10-watch.gdb ./10-refcount-uaf.plain
# -l watches the address, not the expression, so the watchpoint survives
# frames returning and threads switching. Each hit prints a short backtrace.
set pagination off
break control_ready
run
watch -l session_table[7]->refcnt
commands
bt 3
continue
end
continue
