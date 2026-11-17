#!/bin/bash
# check.sh — match captures/<id>.txt against expect.tsv (inside the mdg-lab container).
# usage: ./check.sh [id ...]              (no ids = every id in expect.tsv)
# Prints PASS <id> or FAIL <id>: <reason> per id; exits 1 on any miss.
# Regexes are POSIX ERE and see only the capture body, never the # src / # cmd header.
set -u
cd "$(dirname "$0")" || exit 2

TSV=expect.tsv
exp() { grep -v -e '^#' -e '^[[:space:]]*$' "$TSV"; }

if [ $# -eq 0 ]; then
  ids=()  # bash 3.2 (macOS) has no mapfile
  while IFS= read -r id; do ids+=("$id"); done < <(exp | cut -f1 | awk '!seen[$0]++')
else
  ids=("$@")
fi

rc=0
for id in ${ids[@]+"${ids[@]}"}; do
  f=captures/$id.txt
  if [ ! -f "$f" ]; then echo "FAIL $id: no captures/$id.txt"; rc=1; continue; fi
  body=$(awk 'h && /^# (src|cmd) /{next} {h=0; print}' h=1 "$f")
  n=0; miss=0
  while IFS=$'\t' read -r eid re; do
    [ "$eid" = "$id" ] || continue
    n=$((n + 1))
    printf '%s\n' "$body" | grep -Eq -- "$re" || { echo "FAIL $id: missing /$re/"; miss=1; }
  done < <(exp)
  if [ $n -eq 0 ]; then echo "FAIL $id: no expectations in $TSV"; miss=1; fi
  if [ $miss -eq 0 ]; then echo "PASS $id"; else rc=1; fi
done
exit $rc
