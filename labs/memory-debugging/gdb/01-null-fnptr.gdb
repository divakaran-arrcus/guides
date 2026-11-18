# 01-null-fnptr.gdb — pc is 0: the caller is the return address on top of the stack.
#   gdb -q -batch -nx -x gdb/01-null-fnptr.gdb ./01-null.plain cores/01-null.plain-fnptr.core
set pagination off
bt
p $pc
x/gx $rsp
info symbol *(long*)$rsp
