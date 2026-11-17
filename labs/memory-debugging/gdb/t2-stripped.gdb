# t2-stripped.gdb — a core from a stripped binary, before and after its symbols.
#   make core-10-refcount-uaf V=stripped
#   mkdir -p cores/nodebug && cp 10-refcount-uaf.stripped cores/nodebug/
#   gdb -q -batch -nx -iex 'set debug-file-directory /nonexistent' -x gdb/t2-stripped.gdb \
#       cores/nodebug/10-refcount-uaf.stripped cores/10-refcount-uaf.stripped.core
# The copy in cores/nodebug/ has no .debug next to it, so GDB finds no symbols.
set pagination off
bt
info threads
symbol-file 10-refcount-uaf.debug
bt
thread apply all bt 2
quit
