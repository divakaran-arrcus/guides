# fold-perf.awk — perf script output -> folded stacks ("comm;outer;...;leaf count").
#   perf script -F comm,ip,sym | awk -f fold-perf.awk
# perf script prints each sample as a header line (the comm), then one indented line per frame,
# leaf first, then a blank line. Offsets (+0x1f) are dropped so samples in one function merge.
function flush(   i, s) {
  if (comm == "") return
  s = comm
  for (i = n; i >= 1; i--) s = s ";" f[i]
  count[s]++
  comm = ""; n = 0
}
/^[ \t]*$/ { flush(); next }
/^[ \t]/   { sym = $2; sub(/\+0x[0-9a-f]+$/, "", sym); f[++n] = sym; next }
           { flush(); comm = $1; n = 0 }
END {
  flush()
  for (s in count) print s, count[s] | "sort"
}
