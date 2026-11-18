# 01-null-field.gdb — a fault at 0x18 is a field of a NULL struct pointer.
#   gdb -q -batch -nx -x gdb/01-null-field.gdb ./01-null.plain cores/01-null.plain-field.core
set pagination off
bt
x/i $pc
p $_siginfo._sifields._sigfault.si_addr
ptype /ox struct session
