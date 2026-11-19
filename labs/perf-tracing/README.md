# Performance & tracing lab kit

Small C programs built around `pktd`'s route pipeline (fictional), plus the container and scripts
that produced every terminal capture and flame graph in `perf-tracing-guide.html`. Everything
runs on Linux x86-64 inside the `ptg-lab` container.

## Build the image

```sh
docker build -t ptg-lab - < Dockerfile
```

The Dockerfile copies nothing in, so it needs no build context. The base is Debian 12, pinned by
digest, because its `perf` (linux-perf 6.1) matches a 6.1 kernel; `perf` is built for one
kernel version. If your host runs another kernel, use the `perf` package that matches it.

## Start a shell in it

From this directory:

```sh
docker run -it --rm --hostname pktd-lab -v "$PWD":/lab -w /lab \
  -v /sys/kernel/tracing:/sys/kernel/tracing:ro --ulimit memlock=-1 \
  --cap-add SYS_ADMIN --cap-add SYSLOG --cap-add SYS_PTRACE --security-opt seccomp=unconfined \
  ptg-lab bash
```

## Run a lab

```sh
make 01-hot-path && ./01-hot-path            # the slow version
./01-hot-path --fixed                         # the fix, same binary
./compare.sh 01-hot-path                      # both timelines and the speedup
./06-boss --fix=push,hot                      # the boss level takes a list of fixes
```

Every pipeline lab prints a timeline (stage begin/end relative to the event) and a budget (each
stage's share of the time to converge). `f1-where-time-goes` prints where each thread's wall time
went: on a CPU, waiting in the run queue, or blocked.

| Lab | Planted slowdown | Lens |
|---|---|---|
| `00-baseline` | none | the timeline itself; `bt/stages.bt` |
| `01-hot-path` | a full SPF per link change | `perf stat`, `perf record`, on-CPU flame graph |
| `02-lock` | a "show rib" walker holds `rib_lock` while writing to a slow terminal | off-CPU flame graph (`bt/offcpu.bt`), `bt/lock-wait.bt` |
| `03-syscalls` | one message and one ack per route | `perf trace -s`, `bt/syscalls.bt` |
| `04-sched` | busy-polling threads, pipeline threads at nice 10, 2 CPUs | schedstat, `bt/runqlat.bt`, `perf sched timehist` |
| `05-throttle` | the SPF backoff never resets | the timeline, `bt/hold.bt`, `perf trace -s` |
| `06-boss` | four of the above at once | all of them |

## Scripts

| File | Does |
|---|---|
| `compare.sh <lab> [fix]` | run before and after, print both timelines and the speedup |
| `fold-perf.awk` | `perf script -F comm,ip,sym` → folded stacks for a flame graph |
| `bt/*.bt` | bpftrace programs; `$1` is the lab binary, e.g. `bpftrace -c ./02-lock bt/lock-wait.bt ./02-lock` |
| `symbolize.py <maps>` | names user-space frames in bpftrace output (see below) |
| `fold-bt.awk` | a bpftrace `[comm, ustack]` map → folded stacks |
| `probe.sh` | which perf / eBPF features work in this container on this host |

## `docker run` flags

| Flag | Why |
|---|---|
| `--rm` | Throw the container away after each run; state lives only in `/lab`. |
| `--hostname pktd-lab` | Fixed, neutral hostname, so captures never show the real machine's name. |
| `-v "$PWD":/lab -w /lab` | Work on this directory in place. |
| `-v /sys/kernel/tracing:...:ro` | Tracepoint formats for `perf` and bpftrace. Read-only: nothing is written there. |
| `--ulimit memlock=-1` | bpftrace raises its locked-memory limit; this stops it warning that it can't. |
| `--cap-add SYS_ADMIN` | `perf_event_open` and `bpf()` under `perf_event_paranoid=3` (Debian's strictest level). `CAP_PERFMON` + `CAP_BPF` suffice on hosts with paranoid ≤ 2. |
| `--cap-add SYSLOG` | Lets `perf` read kernel symbol addresses, so kernel frames have names. |
| `--cap-add SYS_PTRACE` | Lets `perf` and bpftrace read the traced process's memory for user stacks. |
| `--security-opt seccomp=unconfined` | Docker's default seccomp profile blocks `perf_event_open` and `bpf()`. |

## Shared hosts: the rules these labs follow

These capabilities let the container load programs into the **host's** kernel. On a host you
share, keep every probe scoped to your own processes:

- No system-wide `perf` (`-a`, `perf sched record`, `perf top`): attach to a command or a PID.
- bpftrace: USDT probes and uprobes on your own binary only. A uprobe on a shared library such as
  libc traps in every process on the host that uses it.
- Tracepoints and kprobes fire for every thread on the host, so filter in the probe (these labs
  use the `pktd-` thread names) and arm them only between the lab's own USDT probes.
- Every run is bounded: the scripts exit at `pktd:converged`, and `run-capture.sh` adds a timeout.
- bcc tools (`offcputime`, `runqlat`, …) take a PID, but inside a container the PID you know is not
  the one the kernel sees; without a usable filter they trace the whole host, so these labs don't
  use them.

## Why `symbolize.py` exists

bpftrace resolves a user-space address by reading `/proc/<pid>/maps` of the traced process. In a
container it only knows the host's PID for that process, which does not exist in the container's
`/proc`, so every frame prints as `[unknown]`. The pipeline labs save their own memory map when
`PKTD_MAPS=<file>` is set; `symbolize.py` uses it and `addr2line` to name each frame.
`--pid=host` would also fix it, but it shows the container every process on the host.

## Captures (maintainers)

`captures.tsv` lists each capture: `id`, the source files it depends on, and the shell command.
`run-capture.sh [id …]` runs rows in the container and writes `captures/<id>.txt`. Each file
starts with `# src <file> <sha256>` and `# cmd <command>` header lines. `check.sh [id …]` matches
each capture against its regexes in `expect.tsv`. Timings vary between runs, so the regexes check
shape (which stage dominates, the speedup) rather than numbers.

`remote.sh [id …]` drives all of it on a remote Docker host from your workstation. Set
`LAB_HOST=user@host` first. It syncs this directory to `~/ptg-lab/`, builds the image, captures,
checks, syncs `captures/` back and runs the repo's leak gate over them. `remote.sh --probe` runs
`probe.sh` there.

## Caveats

- **x86-64 Linux only.** The captures come from a 16-vCPU KVM guest with a virtual PMU and a
  6.1 kernel. Without a PMU (many cloud VMs), `perf stat` shows `<not supported>` for cycles and
  instructions; use `-e cpu-clock` for sampling.
- **`perf lock contention -b`** needs a `perf` built with BPF skeletons; Debian 12's isn't, so
  lock waits come from `bt/lock-wait.bt`.
- **Files made by the container are root-owned on a Linux host.** `remote.sh` fixes that; by hand:
  `docker run --rm -v "$PWD":/lab ptg-lab chown -R "$(id -u):$(id -g)" /lab`
- **`remote.sh` needs a leak-term list** (`$(git rev-parse --git-common-dir)/info/leak-terms` or
  `$LEAK_TERMS`); without one the leak gate exits 2 and `remote.sh` fails, by design.

<!-- cspell:words bpfcc bpftrace kallsyms kprobes libc memlock offcpu offcputime perfmon pktd runqlat sched schedstat seccomp timehist tracepoint tracepoints uprobe uprobes ustack -->
