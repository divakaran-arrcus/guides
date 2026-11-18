# f3-chunks.gdb — dump the middle chunk's header and first words around free(p).
#   gdb -q -batch -nx -x gdb/f3-chunks.gdb ./f3-chunks.plain
set pagination off
break before_free
break after_free
run
up
echo \n--- before free(p): prev_size, size|flags, then user data ---\n
x/8gx p-16
continue
up
echo \n--- after free(p): user data now holds tcache next + key ---\n
x/8gx p-16
printf "next (mangled) = %#lx\n", *(unsigned long *)p
printf "next (decoded) = 0x%lx   (= (&next >> 12) ^ mangled)\n", ((unsigned long)p >> 12) ^ *(unsigned long *)p
printf "tcache key     = %#lx\n", *(unsigned long *)(p + 8)
continue
