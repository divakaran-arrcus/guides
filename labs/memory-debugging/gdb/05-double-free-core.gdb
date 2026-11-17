# 05-double-free-core.gdb — glibc aborted inside free(): who called it?
#   gdb -q -batch -nx -x gdb/05-double-free-core.gdb ./05-double-free.plain cores/05-double-free.plain.core
set pagination off
bt
