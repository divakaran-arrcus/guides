# 06-uaf-core.gdb — the fault address is the freed job's own address >> 12.
#   gdb -q -batch -nx -x gdb/06-uaf-core.gdb ./06-uaf.plain cores/06-uaf.plain.core
set pagination off
bt
p/x job->buf
p/x (unsigned long)job >> 12
p (unsigned long)job->buf == ((unsigned long)job >> 12)
