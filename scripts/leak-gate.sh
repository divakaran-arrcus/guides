#!/bin/sh
# Leak gate: scan files (or staged additions) for forbidden terms.
# usage: leak-gate.sh --staged | <files...>
# exit: 0 clean, 1 hit, 2 error (missing list/file, grep or git failure)
# Patterns: $LEAK_TERMS, else <git-common-dir>/info/leak-terms. If LAB_HOST (user@addr)
# is set, its two parts are also checked, word-bounded. Matched text is never printed,
# only file:line, so the list cannot leak through this script's output.
set -u

if [ $# -eq 0 ]; then echo "usage: leak-gate.sh --staged | <files...>" >&2; exit 2; fi

if [ -n "${LEAK_TERMS:-}" ]; then
  list=$LEAK_TERMS
else
  common=$(git rev-parse --git-common-dir 2>/dev/null) || { echo "leak-gate: not in a git repo and LEAK_TERMS unset" >&2; exit 2; }
  list=$common/info/leak-terms
fi
[ -r "$list" ] || { echo "leak-gate: term list not readable" >&2; exit 2; }

tmp=$(mktemp -d "${TMPDIR:-/tmp}/leakgate.XXXXXX") || exit 2
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

if [ "$1" = "--staged" ]; then
  [ $# -eq 1 ] || { echo "leak-gate: --staged takes no files" >&2; exit 2; }
  git diff --cached -U0 > "$tmp/diff" || { echo "leak-gate: git diff failed" >&2; exit 2; }
  grep '^+' "$tmp/diff" > "$tmp/in"
  rc=$?
  [ $rc -le 1 ] || { echo "leak-gate: grep failed" >&2; exit 2; }  # 1 = no added lines = clean
  targets="$tmp/in"
else
  for f in "$@"; do
    [ -f "$f" ] && [ -r "$f" ] || { echo "leak-gate: cannot read file: $f" >&2; exit 2; }
  done
  targets=
fi

hit=0
scan() {  # scan <grep-args...> ; maps exit codes, prints file:line only
  grep "$@" > "$tmp/out" 2> "$tmp/err"
  rc=$?
  case $rc in
    0) hit=1
       if [ -n "$targets" ]; then cut -d: -f1 "$tmp/out" | sed "s|^|leak-gate: hit in staged diff, diff line |" >&2
       else cut -d: -f1,2 "$tmp/out" | sed "s|^|leak-gate: hit at |" >&2; fi ;;
    1) : ;;
    *) echo "leak-gate: grep error" >&2; exit 2 ;;
  esac
}

# LAB_HOST parts (user, addr); empty parts are dropped, since -e "" would match every line.
hostargs=
if [ -n "${LAB_HOST:-}" ]; then
  hu=${LAB_HOST%@*}; ha=${LAB_HOST#*@}
  [ -n "$hu" ] && hostargs="$hostargs -e $hu"
  [ -n "$ha" ] && hostargs="$hostargs -e $ha"
fi

if [ -n "$targets" ]; then
  scan -inEf "$list" "$targets"
  # shellcheck disable=SC2086
  [ -n "$hostargs" ] && scan -nwF $hostargs "$targets"
else
  scan -HinEf "$list" "$@"
  # shellcheck disable=SC2086
  [ -n "$hostargs" ] && scan -HnwF $hostargs "$@"
fi
[ $hit -eq 0 ] || exit 1
exit 0
