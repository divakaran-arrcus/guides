#!/usr/bin/env bash
# remote.sh — run lab captures on a remote x86-64 Linux Docker host. Runs on your workstation.
# usage: LAB_HOST=user@host ./remote.sh [capture-id ...]   (no ids = every row of captures.tsv)
#        CAP_TIMEOUT=<s> raises run-capture.sh's per-capture timeout (default 300 s)
#        LAB_HOST=user@host ./remote.sh --rr-probe         (decide RR_CAPS, write .rr-caps)
#
# Capture run, in order:
#   1. rsync this directory to ~/mdg-lab/ on the host
#   2. docker build -t mdg-lab (the Dockerfile copies nothing, so no build context is sent)
#   3. run-capture.sh <ids> && check.sh <ids> in the container ($RR_CAPS from .rr-caps)
#   4. chown /lab back to the host user (uid/gid expanded by the host's shell)
#   5. rsync captures/ back
#   6. leak-gate.sh over every capture, here (the container has no repo, git or term list)
# Only the host's sudo-free Docker is used; nothing on the host is reconfigured.
set -euo pipefail
: "${LAB_HOST:?set LAB_HOST=user@host}"
cd "$(dirname "$0")"

SSH=(ssh -o BatchMode=yes)
RSH="ssh -o BatchMode=yes"
RDIR=mdg-lab
# Expanded by the host's shell after "cd mdg-lab", so $PWD is the host's lab copy.
BASE='--rm --hostname pktd-lab -v "$PWD":/lab -w /lab --ulimit core=0 --cap-add SYS_PTRACE --security-opt seccomp=unconfined'
# Built binaries and cores live only on the host: never uploaded, never deleted by --delete.
KEEP=(--exclude 'cores/' --exclude '*.plain' --exclude '*.asan' --exclude '*.tsan' --exclude '*.msan'
      --exclude '*.pattern' --exclude '*.stripped' --exclude '*.reftrace' --exclude '*.debug')

up()    { rsync -az --delete -e "$RSH" "${KEEP[@]}" ./ "$LAB_HOST:$RDIR/"; }
build() { "${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker build -q -t mdg-lab - < Dockerfile"; }

rr_probe() {
  up; build
  local label flags
  for pair in 'perfmon|--cap-add PERFMON' 'sys_admin|--cap-add SYS_ADMIN' 'privileged|--privileged'; do
    label=${pair%%|*}; flags=${pair#*|}
    echo "== rr probe: $label ($flags)"
    if "${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker run $BASE $flags mdg-lab rr record -n /bin/true"; then
      printf '%s\n%s\n' "$label" "$flags" > .rr-caps
      echo "rr-smoke($label)"; return 0
    fi
  done
  printf 'none\n\n' > .rr-caps
  echo "rr-smoke(none)"
}

if [ "${1:-}" = "--rr-probe" ]; then rr_probe; exit 0; fi

for id in "$@"; do
  [[ $id =~ ^[A-Za-z0-9._-]+$ ]] || { echo "remote.sh: bad capture id: $id" >&2; exit 2; }
done
ids="$*"
caps=$(sed -n 2p .rr-caps 2>/dev/null || true)

up; build
set +e
"${SSH[@]}" "$LAB_HOST" "cd $RDIR && docker run $BASE $caps ${CAP_TIMEOUT:+-e CAP_TIMEOUT=$CAP_TIMEOUT} mdg-lab bash -c './run-capture.sh $ids && ./check.sh $ids'"
run_rc=$?
"${SSH[@]}" "$LAB_HOST" 'cd mdg-lab && docker run --rm -v "$PWD":/lab mdg-lab chown -R "$(id -u):$(id -g)" /lab'
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
