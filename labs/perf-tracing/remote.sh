#!/usr/bin/env bash
# remote.sh — run lab captures on a remote x86-64 Linux Docker host. Runs on your workstation.
# usage: LAB_HOST=user@host ./remote.sh [capture-id ...]   (no ids = every row of captures.tsv)
#        CAP_TIMEOUT=<s> raises run-capture.sh's per-capture timeout (default 300 s)
#        LAB_HOST=user@host ./remote.sh --probe            (which perf / eBPF features work there)
#
# Capture run, in order:
#   1. rsync this directory to ~/ptg-lab/ on the host
#   2. docker build -t ptg-lab (the Dockerfile copies nothing, so no build context is sent)
#   3. run-capture.sh <ids> && check.sh <ids> in the container
#   4. chown /lab back to the host user (uid/gid expanded by the host's shell)
#   5. rsync captures/ back
#   6. leak-gate.sh over every capture, here (the container has no repo, git or term list)
# Only the host's sudo-free Docker is used; nothing on the host is reconfigured. The container
# flags (see README.md) let perf and bpftrace observe this container's own processes.
set -euo pipefail
: "${LAB_HOST:?set LAB_HOST=user@host}"
cd "$(dirname "$0")"

SSH=(ssh -o BatchMode=yes)
RSH="ssh -o BatchMode=yes"
RDIR=ptg-lab
# Expanded by the host's shell after "cd ptg-lab", so $PWD is the host's lab copy.
BASE='--rm --hostname pktd-lab -v "$PWD":/lab -w /lab -v /sys/kernel/tracing:/sys/kernel/tracing:ro --ulimit memlock=-1 --cap-add SYS_ADMIN --cap-add SYSLOG'
LABS='f1-where-time-goes 00-baseline 01-hot-path 02-lock 03-syscalls 04-sched 05-throttle 06-boss'
# Built binaries and perf data live only on the host: never uploaded, never deleted by --delete.
KEEP=(--exclude 'perf.data*' --exclude '.*.sh')
for l in $LABS; do KEEP+=(--exclude "/$l"); done

up()    { rsync -az --delete -e "$RSH" "${KEEP[@]}" ./ "$LAB_HOST:$RDIR/"; }
build() { "${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker build -q -t ptg-lab - < Dockerfile"; }

if [ "${1:-}" = "--probe" ]; then
  up; build
  "${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker run $BASE ptg-lab ./probe.sh"
  exit 0
fi

for id in "$@"; do
  [[ $id =~ ^[A-Za-z0-9._-]+$ ]] || { echo "remote.sh: bad capture id: $id" >&2; exit 2; }
done
ids="$*"
if [ -n "${CAP_TIMEOUT:-}" ]; then
  [[ $CAP_TIMEOUT =~ ^[1-9][0-9]*$ ]] || { echo "remote.sh: CAP_TIMEOUT must be a positive integer (seconds), got: $CAP_TIMEOUT" >&2; exit 2; }
fi

up; build
set +e
"${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker run $BASE ${CAP_TIMEOUT:+-e CAP_TIMEOUT=$CAP_TIMEOUT} ptg-lab bash -c './run-capture.sh $ids && ./check.sh $ids'"
run_rc=$?
"${SSH[@]}" "$LAB_HOST" 'cd ptg-lab && docker run --rm -v "$PWD":/lab ptg-lab chown -R "$(id -u):$(id -g)" /lab'
set -e
mkdir -p captures
rsync -az -e "$RSH" "$LAB_HOST:$RDIR/captures/" captures/

gate=$(git rev-parse --show-toplevel)/scripts/leak-gate.sh
set +e
"$gate" captures/*.txt
leak_rc=$?
set -e
[ $leak_rc -eq 0 ] || { echo "remote.sh: leak gate failed (exit $leak_rc)" >&2; exit 1; }
exit $run_rc
