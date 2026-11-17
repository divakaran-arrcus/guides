# Design Spec — Memory Debugging from First Principles (GDB, Sanitizers, Valgrind, rr)

**Status:** v3.1 — **approved by author 2026-11-17**. Approach A; builder check done; capture via Docker on a private x86-64 host (decided); perf kept out except for see-also mentions. v3.1 folds in plan-review R1: `pktd` struct layout fixed, rr capability probe added, lab-kit reference page added
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-11-17
**Output artifact:** `memory-debugging-guide.html` + `labs/memory-debugging/` *(names are Claude's call — see §12)*

---

## 1. Purpose (the why)

Crashes and races in long-running, multi-threaded C daemons are expensive to chase. The crash often happens far away from the cause. Sometimes all you have is one core file from a production run that never happens again. The tools (GDB, ASan/UBSan/LSan/MSan, TSan, Valgrind, rr) are each well documented on their own, but nobody explains *why* they work. Nobody shows which tool fits which situation, or how to work a bug when you can't rebuild and rerun.

This guide exists so that:
1. **The author** gets sharper at chasing crashes and races in multi-threaded C daemons, both when the bug reproduces and when there is only a core.
2. **Other engineers** can learn memory debugging from scratch, or sharpen skills they already have.

## 2. Audience

- **Primary:** the author, an experienced systems/networking engineer who works on multi-threaded C daemons.
- **Secondary:** engineers at any level. Beginners need to be able to start from zero; experienced readers need depth.
- **How one guide serves both:** a single spine that goes from zero to expert. Each chapter follows plain-language memory model → tool → worked bug. Expert material lives in collapsible **"Deep dive"** boxes, which beginners can skip and experts can jump to. *(Claude's call: two separate tracks were rejected because they duplicate content and the beginner track goes stale.)*

## 3. Goals / non-goals / anti-goals

**Goals**
- A reader can explain, from the memory model up, *why* each tool catches what it catches and what it misses.
- A reader can handle a crash in **both modes**:
  - **Path A, "it reproduces":** rebuild with the right instrumentation, trigger the bug, and read the tool's report.
  - **Path B, "one core, no repro":** get a usable core and matching symbols, then do forensics in GDB alone.
- A reader can work a **far-from-cause refcount use-after-free** end to end: core forensics, ASan, watchpoint or rr reverse execution, then design changes that make the next one cheap to find.
- A reader can follow along at a keyboard: every bug is a real, compilable lab with real captured output.

**Non-goals**
- Not a C tutorial; readers are assumed to know C and pointers.
- Not kernel debugging (kdump, crash, KASAN), embedded/JTAG, or Windows tooling.
- Not performance profiling beyond memory growth: no perf or flamegraph chapter. perf is a CPU and performance tool. Its memory features (`perf c2c` for false sharing, `perf mem`) address performance, not correctness, so they get a one-line "see also" in the reference section only. *(Decided with the author.)*
- Not C++-specific material (vtables, exceptions, smart pointers), apart from a one-line note where a tool differs.
- No exploit development. Heap internals are taught for *forensics*, not exploitation.

**Anti-goals (failure even if it "works")**
- **Invented tool output.** Every report, backtrace and GDB transcript must come from a real run (one labelled exception: rr, see §9).
- **A tool-manual tour.** Flag lists without the "why it works" are a failure; the foundations exist to prevent that.
- **Path B as an afterthought.** Core-only forensics must get the same weight as sanitizer workflows.
- **Any employer, product, internal tool or real-incident detail.** The repo is public (see §9 leak gate).

## 4. Scope of tools

- **Core:** GDB (live and on cores), ASan, UBSan, LSan, MSan, TSan, Valgrind (Memcheck, Helgrind, DRD, Massif), rr.
- **Getting cores and symbols:** `ulimit -c`, `core_pattern`, `systemd-coredump`/`coredumpctl`, `gcore`, build-id, split debuginfo (`objcopy --only-keep-debug`, `.gnu_debuglink`), debuginfod.
- **Brief mentions:** HWASan, `MALLOC_CHECK_`/`MALLOC_PERTURB_`, glibc `mtrace`, Electric Fence, heaptrack, `-ftrivial-auto-var-init=pattern`, `_FORTIFY_SOURCE`.
- **Platform:** Linux, **x86-64**, glibc, GCC and Clang. The calling convention, registers and allocator internals are taught for x86-64/glibc. The cheat sheet includes a GDB↔LLDB mapping table and a one-line note on how macOS differs (Valgrind doesn't run there). *(Claude's call: the target daemons run on Linux.)*

## 5. Spine and running example

**Spine:** the memory model → why each tool works → the same bug solved in both situations.

**Running example:** a fictional multi-threaded C daemon, **`pktd`**: a worker thread pool, a shared session table protected by a mutex, a refcounted buffer/session pool, and a control thread. It is labelled illustrative and generic, with nothing taken from real code. Each lab plants one bug in a small, self-contained slice of `pktd`, usually 30–80 lines per file. The boss level can be larger, but it must still build with one compile line.

### 5.1 `pktd` canon (single source of truth)

| Aspect | Canon |
|---|---|
| Threads | 1 main/control thread, N worker threads (default 4), 1 stats thread |
| Shared state | `session_table` (hash of `struct session*`, `pthread_mutex_t table_lock`), a global `stats` struct |
| Objects | `struct session { uint32_t magic; int refcnt; uint32_t id; uint32_t flags; uint64_t created_ns; const struct peer *peer; char name[24]; }` (`peer` sits at offset `0x18`) and `struct pktbuf { uint32_t len; uint32_t port; uint64_t ts; char data[40]; }`. Both are 56 bytes. Their sizes are deliberately **equal**, so a freed `session` chunk gets reused by a `pktbuf` (needed for the boss level). The exact layout is fixed in the implementation plan |
| Ref API | `session_get()` / `session_put()`; `put` frees at 0 |
| Locks | `table_lock`, `stats_lock`; the lock-order rule is table → stats (violated deliberately in bug 9) |

## 6. Structure — chapter outline (approach A: one guide, four parts)

**Front matter**
- **Welcome:** who it's for, how to read it (beginner path vs. Deep-dive boxes), how to run the labs.
- **The Map:** the bug-class × tool × path matrix on one screen, plus a short introduction to `pktd`.

**Part I · Foundations (deep; every section ends with a "→ This is why tool X works" box)**
1. **Process memory layout:** text, data, BSS, heap, mmap regions, thread stacks, guard pages; `/proc/<pid>/maps`. *Hook:* stack overflow = SIGSEGV on a guard page; `info proc mappings` tells you which region a bad pointer falls in.
2. **x86-64 calling convention and stack frames:** argument registers, callee-saved registers, red zone, frame pointer vs. `-fomit-frame-pointer`, return addresses, canaries. *Hook:* why backtraces come out garbled after stack smashing; unwinding by hand from `$rsp`.
3. **The glibc allocator:** chunk header layout, size/prev-in-use bits, tcache/fastbins/small/large/unsorted bins, per-thread arenas, top chunk. *Hook:* reading a corrupted chunk in a raw core; what `malloc(): corrupted top size`, `free(): double free detected in tcache 2` etc. actually mean.
4. **Instrumentation models:** compile-time (sanitizers: shadow memory, redzones, quarantine), dynamic binary translation (Valgrind), recording (rr). *Hook:* speed and accuracy trade-offs, required build flags, and why sanitizers and Valgrind can't be combined. Includes one line explaining that rr records using the same hardware performance counters that `perf` reads, which is why rr needs perf-counter access.
5. **The C memory model:** data race vs. race condition, happens-before, why `volatile` doesn't help, compiler/CPU reordering. *Hook:* TSan's vector clocks catch a race that never actually corrupted anything in your test run.
6. **Atomics, locks, lock ordering, refcounting:** atomic RMW, memory orders (only to the depth needed), mutex internals (`__owner`, `__lock`), correct refcount patterns. *Hook:* the Helgrind/DRD/TSan lock-order detector, and reading mutex owners in a core.

**Part II · Tool primers (about one screen each; "how to drive it", since the why lives in Part I)**
7. GDB on a live process: breakpoints, watchpoints (hardware vs. software), `thread apply all bt`, `x`, `info frame`, scheduler-locking.
8. Getting a usable core and symbols: `core_pattern`, `systemd-coredump`, `gcore`, build-id, split debuginfo, debuginfod; matching a stripped binary.
9. GDB on a core: what is and isn't in a core, reading registers, frames, threads; GDB Python helpers (a short example of walking `session_table`).
10. The ASan family: ASan, UBSan, LSan, MSan; reading the report; `ASAN_OPTIONS` that matter.
11. TSan: reading a race report and a lock-order-inversion report; suppressions; how TSan interacts with custom atomics.
12. Valgrind: Memcheck, Helgrind/DRD, Massif; when Valgrind beats sanitizers (no rebuild, third-party `.so`).
13. rr: record, replay, `reverse-continue`, `watch` + reverse; chaos mode for races.

**Part III · The bug ladder (each rung: an "It reproduces" panel and an "Only a core" panel side by side)**

| # | Bug | Cause → crash distance | Key techniques |
|---|-----|------------------------|----------------|
| 14 | **NULL dereference:** plain `NULL`; `NULL->field` (fault at a small offset such as `0x18`); NULL function pointer (`rip=0`) | Same line | `bt`, `info registers`, `x/i $pc`, mapping the fault offset to a struct field with `ptype /o`, recovering the caller from `[$rsp]` |
| 15 | Uninitialized pointer / stack garbage | Same function | Memcheck, MSan, `-ftrivial-auto-var-init=pattern` |
| 16 | Heap overflow, off-by-one | Nearby | ASan redzones; in a core, a corrupted neighbour chunk header |
| 17 | Stack smashing (`memcpy` into a local buffer) | At function return | Canary abort, destroyed backtrace, manual unwinding |
| 18 | Double free | Close | glibc abort messages, tcache key, ASan |
| 19 | Simple use-after-free | Close | ASan's three stacks (alloc / free / use) |
| 20 | Leak and memory growth | Hours later | LSan, Massif, heaptrack; fragmentation vs. a true leak; `malloc_info` |
| 21 | Data race on a shared counter or flag | Non-deterministic | TSan, Helgrind, happens-before |
| 22 | Lock-order deadlock | Hangs, no crash | `thread apply all bt`, mutex `__owner` → LWP, `gcore` on a live hang |
| 23 | **Boss level: refcount use-after-free, crash far away** | Unrelated code, much later | All four techniques below |

**Boss-level walkthrough (ch 23).** A worker-thread code path calls `session_put()` without having taken a reference. The session reaches refcount 0 and is freed. The allocator recycles the chunk for a `pktbuf` (same size class). Later, unrelated code crashes on what looks like valid data. Solved in escalating order:
1. **Core forensics:** the object doesn't match its type (bad `magic`, plausible-looking `pktbuf` fields). Identify which type now owns the chunk, then reason backwards.
2. **ASan:** quarantine delays reuse, so the use-after-free is caught at the bad access instead of far downstream.
3. **Watchpoint on `refcnt`, or rr `reverse-continue`:** walk back from the crash to the exact wrong decrement.
4. **Design so the next one is cheap to find:** magic/type fields, poison-on-free, a debug-build ring buffer that records each get/put with its caller, and refcount underflow asserts.

**Bridge chapter**
24. **From a one-shot core to a reproducer:** turn a core into a hypothesis, then into a stress or fuzz harness that makes the bug recur (thread jitter, `sched_yield` injection, rr chaos mode, `MALLOC_PERTURB_`).

**Part IV · Reference**
- **Decision matrix:** symptom (SIGSEGV at small address, SIGABRT from malloc, hang, slow growth, wrong data…) → likely bug class → first tool for path A and for path B.
- **Cheat sheet:** GDB commands, sanitizer flags and options, Valgrind tools, rr commands, core/symbol setup.
- **GDB↔LLDB table.**
- **Build-flag matrix:** which flags combine and which conflict (e.g. ASan + TSan can't be used together).
- **Lab kit:** every lab file not shown in full in a chapter, so the guide works as a complete offline kit.
- **Glossary A–Z.**
- **Further reading:** dated, labelled links to primary docs (GCC/Clang sanitizer docs, the Valgrind manual, rr, glibc malloc internals).

Target: about 24 chapters plus front matter and reference. This is the largest guide in the repo, comparable in section count to the EVPN migration guide.

## 7. Labs

- `labs/memory-debugging/`: one `.c` file per lab (plus any shared `pktd.h`), a `Makefile` with targets per lab × instrumentation (`make uaf-asan`, `make race-tsan`, `make null-plain`, …), and a `Dockerfile` for the Linux environment (Ubuntu LTS with GCC, Clang, GDB, Valgrind, rr, heaptrack).
- **Every lab file is embedded whole in the HTML** (`.c`, `pktd.h`, `Makefile`, `Dockerfile`), each in a collapsed full-source block with a copy button, so the guide works as a complete offline lab kit (repo rule: self-contained). Chapters show excerpts inline; the full file sits beneath. The labs are roughly 15 short files, about 30–40 KB in total, so the cost in page size is acceptable. *(Claude's call, from the builder check.)*
- **Sync check:** a small script (`scripts/check-lab-sync.mjs`) verifies every embedded full-source block is byte-identical to its file under `labs/`. It runs alongside cspell and the leak gate before every commit.
- **Every piece of tool output in the guide is captured from a real run** in that environment, then trimmed and annotated. Addresses and PIDs will differ on the reader's machine; the guide says so once, up front.
- **Path B labs** crash `pktd` with sanitizers off and keep the core. The reader then works it with GDB alone.
  - The lab instructions give two ways to get the core:
    - **Your own box:** `ulimit -c unlimited` plus a simple `core_pattern`.
    - **Anywhere, including containers on shared hosts:** GDB-generated cores (`make core-<lab>`).
  - The guide explains why the second exists: `core_pattern` is global to the host, so a container can't change it.
- A `README.md` in the labs folder: how to build the container, run a lab, and get a core inside Docker.

## 8. Visual style

- Matches the existing guides' chrome (sidebar nav grouped by Part, dark/light toggle, info boxes, comparison grids, step blocks, tables). Built with the `guide-builder` skill. Has its own accent colour.
- **Signature visual components** (the equivalent of packet walks): memory-layout diagrams (process map, stack frame, chunk header with bit fields), ASan shadow-byte maps with redzones, happens-before/vector-clock diagrams, a lock-order cycle graph, and the boss-level timeline (get/put events across threads → free → reuse → crash).
- Annotated terminal blocks: real output with numbered callouts.
- "It reproduces" / "Only a core" side-by-side panels for every rung of the ladder.
- Collapsible **Deep dive** boxes.
- Diagrams use the existing Mermaid → inline SVG pipeline (`diagrams/<slug>/`, `scripts/build-diagrams.mjs`) where Mermaid fits. Byte- and bit-level layouts (chunk headers, shadow memory) are hand-drawn inline SVG/HTML tables.

## 9. Constraints

- Single self-contained HTML; no runtime dependencies; works offline.
- cspell clean (add tool and term words such as `tcache`, `redzone`, `debuginfod` to `cspell.json`).
- **Leak gate (hard, before every commit):** `scripts/leak-gate.sh` must exit 0 on the guide, labs, captures, README, spec and plan. It reads the patterns from `$(git rev-parse --git-common-dir)/info/leak-terms`, which works in worktrees too, and treats a missing list as a failure. Examples are invented, never mined from real code, cores or incidents.
- **Capture environment (decided).** All captures run on a private x86-64 Linux build host the author provided. Its address and login are kept out of the repo; they are in the author's private notes and given to the builder directly. Probed on 2026-11-17:
  - Debian 12, kernel 6.1, a KVM guest on an Intel Xeon (Skylake). The virtual PMU is exposed (`arch_perfmon`, hardware `cpu` events), so rr should work.
  - Docker is usable without sudo. Host tools: GCC 12 only; no Clang, GDB, Valgrind or rr.
  - `perf_event_paranoid=3`, `ptrace_scope=0`, `ulimit -c 0`. `core_pattern` pipes to a site-specific handler.
- **No host system settings are changed.** The host is shared, so all captures run inside the lab container from `labs/memory-debugging/Dockerfile`, built on that host:
  - Run with `--cap-add SYS_PTRACE --security-opt seccomp=unconfined --ulimit core=0`.
  - For rr's perf-counter access, probe in order `--cap-add PERFMON`, then `--cap-add SYS_ADMIN`, then `--privileged`, and use the first that works. Debian's paranoid=3 patch may require `SYS_ADMIN`. All three options stay confined to the container.
  - Verify early with `rr record /bin/true`. If that fails, rr output falls back to "representative, visibly labelled" — the only permitted exception to real captures.
  - `core_pattern` is global to the host and can't be changed from a container. Path-B lab cores are therefore produced with `gcore` / GDB `generate-core-file` at the faulting instruction (`gdb -batch -ex run -ex generate-core-file`), or by `gcore` on a live hang. These are real cores of the crashed state.
  - `core_pattern` and `systemd-coredump` are still taught. Any `coredumpctl` output in the guide is labelled representative unless captured elsewhere.
- **Never quote the host's `core_pattern` handler path** or anything else identifying the host. It falls under the leak gate.
- Review rounds captured as `REVIEW-memory-debugging-guide-R<n>.md`.
- `README.md` gains a new "Systems Debugging" table with this guide.

## 10. Success criteria

- The author can take a fresh core from a stripped multi-threaded C binary and, using only the guide's path-B workflow, find the faulting object, its type, and the thread and lock state.
- Every ladder rung has both panels with real (or, for rr only, visibly labelled) output, and every lab builds and reproduces as described.
- Every Part I section ends with its tool hook, and every tool in Part II points back to the Part I section that explains it.
- The boss level is solved all four ways, and its design takeaways appear in the cheat sheet.
- Leak gate returns zero hits; cspell is clean.

## 11. Approaches considered

| Option | Shape | Trade-off | Decision |
|---|---|---|---|
| **A** | One guide: foundations → tool primers → bug ladder (A/B panels) → reference | Largest file in the repo; keeps each bug's A/B pair together; one place to look things up | **Chosen** |
| B | Two-part series split by path (repro vs. core-only) | Smaller files, reads like a course, but splits every bug across two files | Rejected |
| C | One guide organized by tool | Best as a lookup reference, worst for learning; the boss level has nowhere natural to go | Rejected (Part II + reference recover most of its value) |

## 12. Calls Claude made (review these)

- File names: `memory-debugging-guide.html` and `labs/memory-debugging/`.
- Deep-dive boxes rather than two tracks.
- Linux/x86-64/glibc only, plus a GDB↔LLDB table.
- rr included; it fills the gap for races that won't reproduce.
- Brief-mention list (HWASan, Electric Fence, heaptrack, …).
- The `pktd` canon (§5.1), including equal `session`/`pktbuf` sizes for the boss level.
- Part II tool primers as a separate part (not folded into the ladder).
- The bridge chapter (24) placed after the ladder.
- The rr "representative output" fallback.
- Whole lab files embedded (collapsed) plus a byte-identical sync check (§7).

## 13. Left to the builder

Accent colour; exact diagram visuals; the per-chapter section breakdown; exact lab file names and Makefile target names; the GCC/Clang/Ubuntu versions (pin them and state them in the guide); how much glibc malloc source to quote (keep it short; link to the source); build order (detailed in the implementation plan).
