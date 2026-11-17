# 05-double-free-core.gdb — glibc aborted inside free(): who called it, and what is in the chunk?
#   gdb -q -batch -nx -x gdb/05-double-free-core.gdb ./05-double-free.plain cores/05-double-free.plain.core
set pagination off
bt
echo \n--- frame 8: the pointer free() was given ---\n
frame 8
p mem
echo \n--- frame 9: the lab code, mid-loop ---\n
frame 9
p i
p session_table[i]
echo \n--- the chunk: tcache next (safe-linked), then the key ---\n
x/2gx session_table[i]
p/x (unsigned long)session_table[i] >> 12
p/x *session_table[i]
