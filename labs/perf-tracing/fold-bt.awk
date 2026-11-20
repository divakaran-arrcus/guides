# fold-bt.awk — a bpftrace map keyed by [comm, ustack(perf)] -> folded stacks.
#   bpftrace ... bt/offcpu.bt ... | awk -v map=@offcpu_us -f fold-bt.awk
# bpftrace prints each entry as "@map[comm, " then one indented frame per line (leaf first,
# "addr sym+off (path)"), then "]: value". Offsets are dropped so one function merges.
BEGIN { if (map == "") map = "@offcpu_us" }
index($0, map "[") == 1 {
  comm = substr($0, length(map) + 2); sub(/, *$/, "", comm); n = 0; inmap = 1; next
}
inmap && /^\]: / {
  s = comm
  for (i = n; i >= 1; i--) s = s ";" f[i]
  total[s] += substr($0, 4)
  inmap = 0; next
}
inmap && /^[ \t]/ { sym = $2; sub(/\+[0-9]+$/, "", sym); f[++n] = sym; next }
END { for (s in total) print s, total[s] | "sort" }
