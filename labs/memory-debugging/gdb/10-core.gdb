# 10-core.gdb — the boss crash, read from its core: whose bytes are in session 7?
#   gdb -q -batch -nx -x gdb/10-core.gdb ./10-refcount-uaf.plain cores/10-refcount-uaf.plain.core
set pagination off
source pktd-gdb.py
bt
frame function table_dump
p *s
p/x s->magic
p *(struct pktbuf *)s
pktd-sessions
