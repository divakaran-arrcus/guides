# 09-deadlock-core.gdb — a gcore of the hung process: who waits, who owns.
#   gdb -q -batch -nx -x gdb/09-deadlock-core.gdb ./09-deadlock.plain cores/09.<pid>
set pagination off
info threads
thread apply all bt
p table_lock.__data.__owner
p stats_lock.__data.__owner
