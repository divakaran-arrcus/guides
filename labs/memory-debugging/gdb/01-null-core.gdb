# 01-null-core.gdb — open the plain NULL-deref core: where, what address, and which pointer.
#   gdb -q -batch -nx -x gdb/01-null-core.gdb ./01-null.plain cores/01-null.plain-plain.core
set pagination off
bt
p $_siginfo._sifields._sigfault.si_addr
frame 1
p s
