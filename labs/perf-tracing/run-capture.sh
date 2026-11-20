#!/bin/bash
# run-capture.sh — run rows of captures.tsv and write captures/<id>.txt (inside the ptg-lab container).
# usage: ./run-capture.sh [id ...]        (no ids = every row)
#
# Each capture file is:
#   # src <file> <sha256>     one line per source listed in the row ("-" = none)
#   # cmd <command>
#   <combined stdout+stderr of the command>
# C0 control bytes other than \t \n \r are stripped, so ANSI colour and stray
# binary bytes never reach the guide. The command's exit status is ignored:
# check.sh decides whether a capture is good.
set -u
cd "$(dirname "$0")" || exit 2

TSV=captures.tsv
TIMEOUT=${CAP_TIMEOUT:-300}   # seconds per capture, so a hung lab cannot stall a run

rows() { grep -v -e '^#' -e '^[[:space:]]*$' "$TSV"; }

# Reject unknown ids before running anything.
for want in "$@"; do
  rows | cut -f1 | grep -qxF -- "$want" || { echo "run-capture: unknown id: $want" >&2; exit 2; }
done

mkdir -p captures
while IFS=$'\t' read -r id srcs cmd; do
  if [ $# -gt 0 ]; then
    hit=0; for want in "$@"; do [ "$want" = "$id" ] && hit=1; done
    [ $hit -eq 1 ] || continue
  fi
  out=captures/$id.txt
  if [ "$srcs" != "-" ]; then
    IFS=',' read -ra files <<< "$srcs"
    for f in "${files[@]}"; do
      [ -f "$f" ] || { echo "run-capture: $id: source not found: $f" >&2; exit 2; }
    done
  fi
  {
    if [ "$srcs" != "-" ]; then
      IFS=',' read -ra files <<< "$srcs"
      for f in "${files[@]}"; do
        echo "# src $f $(sha256sum "$f" | cut -d' ' -f1)"
      done
    fi
    echo "# cmd $cmd"
    timeout -k 5 "$TIMEOUT" bash -c "$cmd" < /dev/null 2>&1 | tr -d '\000-\010\013\014\016-\037'
  } > "$out" || exit 2
  echo "captured $id"
done < <(rows)
