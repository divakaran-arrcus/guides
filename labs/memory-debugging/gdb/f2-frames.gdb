# f2-frames.gdb — stop in the innermost frame and look at the stack.
#   gdb -q -batch -nx -x gdb/f2-frames.gdb ./f2-frames.plain
set pagination off
break checksum
run
bt
info frame
x/4gx $rsp
disassemble
continue
