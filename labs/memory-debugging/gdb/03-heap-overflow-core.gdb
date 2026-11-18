# 03-heap-overflow-core.gdb — free() aborted: look at the chunk header it read.
#   gdb -q -batch -nx -x gdb/03-heap-overflow-core.gdb ./03-heap-overflow.plain cores/03-heap-overflow.plain.core
set pagination off
bt
frame function main
echo \n--- b's chunk header: prev_size (a's last 8 chars), then size|flags ---\n
x/4gx b-16
x/s a
