# 01-null-core.gdb — open the plain NULL-deref core: where, and which pointer.
#   gdb -q -batch -nx -x gdb/01-null-core.gdb ./01-null.plain cores/01-null.plain-plain.core
set pagination off
bt
frame 1
p s
