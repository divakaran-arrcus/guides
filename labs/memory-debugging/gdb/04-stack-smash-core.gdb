# 04-stack-smash-core.gdb — the canary check fired: is the return address still sane?
#   gdb -q -batch -nx -x gdb/04-stack-smash-core.gdb ./04-stack-smash.plain cores/04-stack-smash.plain.core
set pagination off
bt
frame function ctrl_set_hostname
echo \n--- the smashed frame: host[16], canary, saved rbp, saved rip ---\n
x/16gx $rsp
echo \n--- candidate return addresses ---\n
info symbol *(long*)($rbp+8)
info symbol $pc
