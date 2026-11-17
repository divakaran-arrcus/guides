# Memory-debugging lab kit

Small C programs, one per bug class, plus the container and scripts that produced every
terminal capture in the guide. Everything runs on Linux x86-64 inside the `mdg-lab` container.

## Build the image

```sh
docker build -t mdg-lab - < Dockerfile
```

The Dockerfile copies nothing in, so it needs no build context. The base image is pinned by
digest: gcc 13, clang 18 (with its sanitizer runtimes), gdb, valgrind, rr and heaptrack.

## Start a shell in it

From this directory:

```sh
docker run -it --rm --hostname pktd-lab -v "$PWD":/lab -w /lab \
  --ulimit core=0 --cap-add SYS_PTRACE --security-opt seccomp=unconfined \
  mdg-lab bash
```

Add `--cap-add PERFMON` (or `--cap-add SYS_ADMIN`, or `--privileged`) if you want to use rr.
See the flag table below.

## Run a lab

```sh
make 06-uaf.asan && ./06-uaf.asan
```

`make <lab>.<variant>` builds one lab in one variant: `plain`, `asan`, `tsan`, `msan`,
`pattern`, `stripped` or `reftrace`. The `Makefile` header lists the exact flags.

## Get a core

```sh
make core-01-null ARGS=field
```

This runs the lab under GDB and has GDB write `cores/01-null.plain-field.core` at the fault.
Pick another build with `V=<variant>`. The target fails if the program exits normally.
Open the core with `gdb ./01-null.plain cores/01-null.plain-field.core`.

### Why GDB writes the core, not the kernel

- **A container can't change `core_pattern`.** `/proc/sys/kernel/core_pattern` is global to
  the host kernel, not namespaced. Inside a container it is read-only. On a host where it pipes
  to a handler (systemd-coredump, apport and the like), a kernel core from the container goes to
  the *host's* handler. It never lands in your working directory.
- **So the container runs with `--ulimit core=0`.** With a file `core_pattern` the kernel then
  writes no core. With a pipe the kernel ignores the limit: it still starts the host's handler and
  passes it the limit (`%c`). A limit-honouring handler such as systemd-coredump keeps nothing;
  other handlers may not. Either way, no stray core lands in your working directory.
  `generate-core-file` is GDB writing a file through ptrace, so the limit doesn't apply to it.

## `docker run` flags

| Flag | Why |
|---|---|
| `--rm` | Throw the container away after each run; state lives only in `/lab`. |
| `--hostname pktd-lab` | Fixed, neutral hostname, so captures never show the real machine's name. |
| `-v "$PWD":/lab -w /lab` | Work on this directory in place; paths in captures read `/lab/...`. |
| `--ulimit core=0` | No file cores; a limit-honouring pipe handler keeps nothing, though it still runs (see above). GDB-written cores still work. |
| `--cap-add SYS_PTRACE` | Lets gdb attach to running processes and lets ASan/LSan inspect threads. |
| `--security-opt seccomp=unconfined` | Allows `personality()` (gdb disables ASLR) and the perf/ptrace calls rr needs. |
| `--cap-add PERFMON` / `SYS_ADMIN` / `--privileged` | Only for rr: it needs `perf_event_open`. Use the weakest one that works on your host. |

## Captures (maintainers)

`captures.tsv` lists each capture: `id`, the source files it depends on, and the shell command.
`run-capture.sh [id …]` runs rows in the container and writes `captures/<id>.txt`. Each file
starts with `# src <file> <sha256>` and `# cmd <command>` header lines. `check.sh [id …]` matches
each capture against its regexes in `expect.tsv`.

`remote.sh [id …]` drives all of it on a remote Docker host from your workstation. Set
`LAB_HOST=user@host` first. It syncs this directory to `~/mdg-lab/`, builds the image, captures,
checks, syncs `captures/` back and runs the repo's leak gate over them.
`remote.sh --rr-probe` finds the weakest flag that lets `rr record` work on that host. It records
the result in `.rr-caps`, a local file that is not committed.

## Caveats

- **You need an x86-64 Linux host or VM.** The captures come from x86-64, and addresses, frame
  layouts and sanitizer reports are x86-64 specific. On arm64 (Apple Silicon) or under emulated
  Docker they won't match, and rr and Valgrind may not run at all.
- **Files made by the container are root-owned on a Linux host.** `remote.sh` fixes that for
  you. When you run `docker` by hand, finish with the same step, from this directory:

  ```sh
  docker run --rm -v "$PWD":/lab mdg-lab chown -R "$(id -u):$(id -g)" /lab
  ```

- **`remote.sh` needs a leak-term list.** Its last step runs the repo's leak gate over the
  captures. The list is read from `$(git rev-parse --git-common-dir)/info/leak-terms`, or from the
  file named by `$LEAK_TERMS`. Without a readable, non-empty list the gate exits 2 and `remote.sh`
  fails, by design. If you work outside the original project, provide a list of your own private
  terms (hostnames, addresses, logins you don't want published), one pattern per line.
- **`check.sh` normally runs inside the container**, right after `run-capture.sh`. It only reads
  `captures/` and `expect.tsv`, so on a Mac (bash 3.2) it also works from a plain shell, with a list
  of ids or with none.

<!-- cspell:words apport asan coredump heaptrack msan perfmon pktd reftrace seccomp tsan valgrind -->
