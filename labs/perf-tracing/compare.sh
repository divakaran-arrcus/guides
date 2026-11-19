#!/bin/bash
# compare.sh <lab> [fix-args] — run a lab as it is, then with its fix, and print both timelines
# and the speedup.  e.g.  ./compare.sh 01-hot-path           (the fix is --fixed)
#                         ./compare.sh 06-boss --fix=push    (the boss takes --fix=...)
set -eu
lab=$1
fix=${2:---fixed}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
./"$lab" > "$tmp/before"
./"$lab" "$fix" > "$tmp/after"
cat "$tmp/before"
echo "--- with $fix:"
cat "$tmp/after"
b=$(sed -n 's/^converged_ms=//p' "$tmp/before")
a=$(sed -n 's/^converged_ms=//p' "$tmp/after")
awk -v b="$b" -v a="$a" 'BEGIN { printf "speedup=%.1fx  (%.1f ms -> %.1f ms)\n", b / a, b, a }'
