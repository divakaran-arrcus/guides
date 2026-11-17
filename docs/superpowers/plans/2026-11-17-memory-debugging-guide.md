# Memory Debugging Guide Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Load the `guide-builder` skill for the repo's HTML component vocabulary.

**Goal:** Build `memory-debugging-guide.html` ("Memory Debugging from First Principles") and a runnable lab kit in `labs/memory-debugging/`. Every piece of tool output in the guide is captured from a real run and injected by a script, never retyped by hand.

**Architecture:** There are two halves, joined by a sync script.
- **Labs.** Small C programs, each a slice of the fictional `pktd` daemon. A Makefile builds them per instrumentation, and a Docker image pins the toolchain. On the remote x86-64 host, a capture runner inside that image writes each tool's output to `captures/<id>.txt` and checks it against expected signatures.
- **Guide.** Built like the harness-engineering guide:
  - a cloned chrome shell;
  - per-Part HTML fragments in the git-ignored `.build/memory-debugging/`, spliced in by `assemble-guide.mjs`;
  - Mermaid diagrams injected by `build-diagrams.mjs`.
- **The bridge.** A new `scripts/sync-labs.mjs` HTML-escapes lab files and captures (whole or by line range) into marker blocks in the HTML. In `--check` mode it fails on any drift.

**Tech Stack:**
- C: GCC 13 and Clang 18 on Ubuntu 24.04, in Docker.
- GNU Make, POSIX sh, GDB, Valgrind, rr, heaptrack.
- Node ≥ 20 for scripts, tested with `node:test`.
- HTML5 with vanilla CSS/JS.
- `npx @mermaid-js/mermaid-cli@11.4.2` and `npx -y cspell@8.14.4`.

**Spec:** `docs/superpowers/specs/2026-11-17-memory-debugging-guide-design.md` (v3.1, approved). Read §2, §5.1, §6 and §9 before any content task.

**Review history:** `docs/superpowers/plans/REVIEW-2026-11-17-memory-debugging-guide-R1.md` (22 findings, 9 risks) and `…-R2.md` (15 findings, 3 new risks). All of them are folded into this revision. Review files are git-ignored by repo convention and stay local.

## Global Constraints

- **One self-contained HTML file.** No external runtime JS or CSS, except the Google Fonts link the cloned chrome already has. It works offline.
- **The repo is public, so everything goes through `scripts/leak-gate.sh` (Task 1).**
  - **What it checks:**
    - Patterns come from `$(git rev-parse --git-common-dir)/info/leak-terms`. That path also works in a worktree, where `.git` is a file.
    - If `LAB_HOST` is set, it also checks word-bounded for its two parts: the part before `@` and the part after it.
  - **Exit codes:** 0 = clean, 1 = hit, 2 = list missing or grep error.
  - **How to run it:**
    - Before every commit: `scripts/leak-gate.sh --staged` must exit 0.
    - On specific files: `scripts/leak-gate.sh <files…>`.
  - **Keep host details out of the repo.** Never write the capture host's address, login or `core_pattern` handler path into any repo file. Reach the host only through the env var `LAB_HOST` (`user@addr`), which the executor sets from the author's private memory note `memory-debugging-capture-host`.
- **Never change the host.** Don't run `sudo`, `sysctl`, `apt`, or anything that edits `core_pattern` on the capture host.
  - Everything runs in the `mdg-lab` container with these flags:
    - `--rm --hostname pktd-lab -v "$PWD":/lab -w /lab`
    - `--ulimit core=0`, so kernel cores never reach the host's pipe handler. GDB-written cores are unaffected.
    - `--cap-add SYS_PTRACE --security-opt seccomp=unconfined`
    - `$RR_CAPS`, which Task 2 decides: `--cap-add PERFMON`, `--cap-add SYS_ADMIN` or `--privileged`.
  - `remote.sh` ends every run with an in-container `chown -R <host uid>:<gid> /lab`.
- **No invented output.** Every terminal block in the guide is a `<!--CAP:…-->` block filled by `sync-labs.mjs`.
  - There are two exceptions, and both use `<pre class="term representative">` with the visible label "Representative output — not captured":
    - rr output, only if Task 2's rr smoke test fails with all three `RR_CAPS` options;
    - `coredumpctl` output.
- **Platform taught:** Linux x86-64, glibc, GCC/Clang. Toolchain versions come from `captures/00-env.txt`, never from memory.
- **`pktd` canon.** Examples are invented. The canon is spec §5.1, refined here so that offset `0x18` is a pointer field (review M3):
  - `struct peer { uint32_t addr; uint16_t port; }`
  - `struct session { uint32_t magic; int refcnt; uint32_t id; uint32_t flags; uint64_t created_ns; const struct peer *peer; char name[24]; }` — 56 bytes, with `peer` at `0x18`
  - `struct pktbuf { uint32_t len; uint32_t port; uint64_t ts; char data[40]; }` — 56 bytes
  - `_Static_assert` that both sizes are equal and that `offsetof(struct session, peer) == 0x18`
  - The lock order is `table_lock` → `stats_lock`. The threads are 4 workers, 1 control and 1 stats.
- **Accent colour:** `--accent: #f59e0b` (dark) / `#b45309` (light), amber (Claude's call).
- **Content style:** match the harness-engineering guide — a `<div class="card">` per subsection, `<h2>N.k Title</h2>`, `<h4 class="ph">` mini-headings, second person, short sentences.
- **Source of truth:**
  - Until Task 13 Step 2 (the final assemble), the source is the fragments in `.build/`. After that, edit the HTML only.
  - Always run in this order: assemble → `sync-labs --write` → `build-diagrams`.
  - Never re-run the assembler after the final assemble.
- **Workspace.** The main checkout has unrelated uncommitted edits to 13 guides, so execute in a worktree:
  - `git worktree add ../guides-memdbg -b docs/memory-debugging-guide fix/guide-nav-highlight`
  - Copy the spec and this plan into it, and commit them first.
  - Stage explicit paths only, never `git add -A`.
  - The worktree's own ignore file is `$(git rev-parse --git-common-dir)/info/exclude`.
- **Commits:**
  - Prefixes: `feat(memdbg): …` (scripts, labs, shell), `docs(memdbg): …` (content), `fix(memdbg): …`.
  - Each message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- **Regex style.** In `expect.tsv`, patterns are POSIX ERE with a bare `|` for alternation. The `\|` in this document's tables is only Markdown escaping.

## Review Focus

1. **A capture goes stale after its lab is edited**, so line numbers no longer match the source.
   - Pinned in Task 1: each capture header records `# src <file> <sha256>`.
   - `sync-labs --check` fails when a hash differs.
2. **The copy button copies escaped or decorated text.**
   - Pinned in Task 1: a round-trip test on C containing `<`, `>`, `&&`, `"` and `'`.
   - Task 7: the button copies from `code.textContent` only.
3. **Captures leak the host or contain binary garbage.**
   - Pinned in Task 2:
     - `--hostname pktd-lab` (not a likely login name, review R2 on R9) and `/lab` paths;
     - `remote.sh` runs `leak-gate.sh` over every capture on the Mac after the rsync back;
     - `run-capture.sh` strips C0 control bytes except `\t`, `\n` and `\r`.
4. **The boss bug doesn't crash the same way every run.**
   - Pinned in Task 6: the plain run happens 5 times and must give `unique=1`.
   - The crash is a deterministic non-canonical pointer deref, not a garbage string.
5. **Narrow screens.** The A/B panels and wide terminal output scroll the page sideways.
   - Pinned in Task 7's CSS: panels stack below 768px, and `pre` scrolls inside its box.
   - Also checked by the 390px screenshot gate in Task 13.

---

## File Structure

```
labs/memory-debugging/
  Dockerfile            pinned Ubuntu 24.04 digest + toolchain (incl. libclang-rt-18-dev)
  Makefile              <lab>.<variant> binaries; core-<lab> (V=, ARGS=)
  README.md             build image, run a lab, get a core (incl. container note)
  pktd.h                shared pktd types + session API (canon above)
  captures.tsv          id <TAB> sources(comma) <TAB> shell command
  expect.tsv            id <TAB> ERE   (one or more lines per id)
  run-capture.sh        runs rows -> captures/<id>.txt (# src / # cmd headers, control bytes stripped)
  check.sh              greps expect.tsv vs captures + leak-gate over captures; exit 1 on any miss
  remote.sh             (Mac) rsync -> build -> run-capture -> check -> chown -> rsync back
  pktd-gdb.py           GDB helper: `pktd-sessions` walks session_table
  f1-layout.c f2-frames.c f3-chunks.c
  01-null.c 02-uninit.c 03-heap-overflow.c 04-stack-smash.c 05-double-free.c
  06-uaf.c 07-leak.c 08-race.c 09-deadlock.c 10-refcount-uaf.c
  gdb/*.gdb             batch command files
  captures/*.txt        committed real output
scripts/
  leak-gate.sh          leak + host-string gate (see Global Constraints)
  lib/guide-path.mjs    resolveGuideHtml(root, slug)
  sync-labs.mjs         --write | --check  (SRC + CAP markers, optional lines=)
  test/sync-labs.test.mjs, test/leak-gate.test.mjs
  assemble-guide.mjs    modify: resolveGuideHtml + --out <file>
  build-diagrams.mjs    modify: ROOT from import.meta.url + resolveGuideHtml
diagrams/memory-debugging/_registry.mjs + 7 *.mmd
memory-debugging-guide.html
.build/memory-debugging/{part0,part1,part2,part3a,part3b,ref}.html   (git-ignored)
README.md, cspell.json  modify
REVIEW-memory-debugging-guide-R1.md   (local only — git-ignored by repo convention)
```

### Marker formats (fixed)

- **Source:**
  ```
  <details class="src"><summary>06-uaf.c — full source</summary><button class="copy">Copy</button><pre><code><!--SRC:06-uaf.c--><!--/SRC:06-uaf.c--></code></pre></details>
  ```
  - Paths are relative to `labs/memory-debugging/`, for example `SRC:gdb/10-rr.gdb`.
  - For an inline excerpt, use `<pre class="excerpt"><code><!--SRC:10-refcount-uaf.c lines=40-62--><!--/SRC:10-refcount-uaf.c--></code></pre>`. This keeps excerpts under `--check` too.
- **Capture:**
  ```
  <pre class="term"><code><!--CAP:06-uaf.asan lines=1-18,40-44--><!--/CAP:06-uaf.asan--></code></pre>
  ```
  - Follow it with `<ol class="callouts">`. Each item refers to "line N" of the block as shown.
- **Rules for both:**
  - `lines` is optional.
  - Gaps between ranges render as a single line `⋯`.
  - **CAP only:** the leading `# src` / `# cmd` header lines are never shown, and `lines=` counts from the first line after them.
  - **SRC blocks are byte-identical to the file.** Nothing is stripped, so comments in the Makefile, shell, Python and `.gdb` files stay, and `lines=` counts file lines (review R2-M3).
  - The same id may appear in several places. Each open marker pairs with the **first close marker after it**.

### Section ids (fixed)

| Fragment | Ids (sidebar group) |
|---|---|
| part0 | `welcome` (class `section active`), `map` (Front Matter) |
| part1 | `f1-layout`, `f2-frames`, `f3-allocator`, `f4-instrumentation`, `f5-memory-model`, `f6-atomics-locks` (Part I · Foundations) |
| part2 | `t1-gdb-live`, `t2-cores-symbols`, `t3-gdb-core`, `t4-asan`, `t5-tsan`, `t6-valgrind`, `t7-rr` (Part II · Tool Primers) |
| part3a | `b1-null`, `b2-uninit`, `b3-heap-overflow`, `b4-stack-smash`, `b5-double-free` (Part III · The Bug Ladder) |
| part3b | `b6-uaf`, `b7-leak`, `b8-race`, `b9-deadlock`, `b10-refcount-uaf` (Part III, cont.), `x1-core-to-repro` (Bridge) |
| ref | `ref-decision`, `ref-cheat`, `ref-gdb-lldb`, `ref-flags`, `ref-labkit`, `ref-glossary`, `ref-reading` (Part IV · Reference) |

That is **33 sections**.
- Chapters are numbered 1–24 in `<h1>`, in spec §6 order: f1–f6 = 1–6, t1–t7 = 7–13, b1–b10 = 14–23, x1 = 24.
- Reference pages are unnumbered.
- `ref-labkit` is new (review M9). It embeds every lab file not already shown in full elsewhere.

### Make variants (fixed; `make <lab>.<variant>`)

| Variant | Flags |
|---|---|
| `plain` | `gcc -O0 -g -fno-omit-frame-pointer -pthread` |
| `asan` | plain + `-fsanitize=address,undefined` |
| `tsan` | `gcc -O1 -g -fsanitize=thread -pthread` |
| `msan` | `clang -O1 -g -fsanitize=memory -fsanitize-memory-track-origins -pthread` |
| `pattern` | plain + `-ftrivial-auto-var-init=pattern` (GCC fills with **0xFE** bytes; the guide notes that Clang uses 0xAA) |
| `stripped` | `plain` → `objcopy --only-keep-debug` → `<lab>.debug`, `strip`, `--add-gnu-debuglink` |
| `reftrace` | plain + `-DPKTD_REFTRACE -rdynamic` (boss lab only) |

`make core-<lab> [V=<variant>] [ARGS=<args>]` (defaults: `V=plain`, no args):
- It runs `gdb -batch -ex "run $(ARGS)" -ex "generate-core-file cores/$*.$(V)$(if $(ARGS),-$(ARGS)).core" ./$*.$(V)`.
- It **fails if the program exits normally**. Check this with `gdb`'s `$_siginfo` being set, or by `test -f` on the core.
- Each lab with a `.core` capture must fault deterministically in its `plain` (or named) variant.

---

### Task 1: Scripts — leak gate, slug resolution, `--out`, `sync-labs.mjs`

**Files:**
- Create: `scripts/leak-gate.sh`, `scripts/lib/guide-path.mjs`, `scripts/sync-labs.mjs`, `scripts/test/sync-labs.test.mjs`, `scripts/test/leak-gate.test.mjs`.
- Modify: `scripts/assemble-guide.mjs`, `scripts/build-diagrams.mjs`.

**Produces:**
- **`scripts/leak-gate.sh [--staged | <files…>]`**
  - Exit codes: 0 = clean, 1 = hit, 2 = error.
  - The `LEAK_TERMS` env var overrides the list path. It exists for tests.
- **`resolveGuideHtml(root: string, slug: string): string`**
  - Returns `<root>/<slug>-guide.html` if that exists, otherwise `<root>/gen-ai-<slug>-guide.html`.
- **`assemble-guide.mjs <slug> [--out <file>]`**
  - Reads the resolved HTML and writes to `--out` if given, otherwise in place.
- **`build-diagrams.mjs`**
  - `ROOT` is derived from `import.meta.url`; the hard-coded path is removed.
  - It uses `resolveGuideHtml`.
- **`node scripts/sync-labs.mjs <slug> --write|--check [--labs <dir>]`**
  - `--labs` defaults to `labs/<slug>`.
  - Exports: `escapeHtml(s)`, `unescapeHtml(s)`, `selectLines(text, spec?)` and `syncHtml(html, labsDir, mode) → {html, errors}`.

- [ ] **Step 0:** Create the worktree (Global Constraints). `leak-gate.sh` doesn't exist yet, so check the plan and spec inline first (review R2-L4):
  ```bash
  grep -inEf "$(git rev-parse --git-common-dir)/info/leak-terms" <spec> <plan>
  ```
  It must exit `1` (no match). Then commit them as `docs(memdbg): design spec and implementation plan`.
- [ ] **Step 1: Write the failing tests.**
  - **`sync-labs.test.mjs`** (temp-dir fixtures):
    - `escapeHtml round-trips C source`: `unescapeHtml(escapeHtml(src)) === src` for `'if (a < b && p->x > 0) puts("ok \'q\'");\n'`.
    - `selectLines`: `"a\nb\nc\nd\ne\n"` with `"1-2,4"` → `"a\nb\n⋯\nd"`. With `undefined`, the input is returned unchanged.
    - `CAP strips only leading # src/# cmd headers`: a capture whose body contains a `# comment` line keeps it.
    - `SRC keeps # lines`: a Makefile with `# comment` lines embeds byte-identical.
    - `write fills SRC and CAP`: CAP excludes the `# src` header.
    - `SRC lines= excerpt is filled and checked`.
    - `repeated id in two places`: both blocks are filled and independent. One has `lines=1-1` and the other is whole.
    - `check flags drifted SRC`: one error naming the file.
    - `check flags stale capture`: the `# src x.c <sha>` header doesn't match, so the error names the capture id.
    - `missing file is an error` in both modes.
    - `write is idempotent`.
    - `resolveGuideHtml prefers <slug>-guide.html`, and falls back to gen-ai when only that exists.
  - **`leak-gate.test.mjs`**, with `LEAK_TERMS` pointing at a temp list containing `zzleakzz`:
    - a clean file → exit 0;
    - a file containing `zzleakzz` → exit 1;
    - `LEAK_TERMS=/nonexistent` → exit 2;
    - `LAB_HOST=lab@10.0.0.9` with a file containing `ubuntu:24.04 --hostname labs` → exit 0 (word-bounded: `lab` must not match `labs`);
    - the same with a file containing ` 10.0.0.9 ` → exit 1;
    - `--staged` in a temp repo where the only staged change is a `git rm` (no `+` lines) → exit 0. The `grep '^+'` that finds nothing must not be read as a hit or an error (review R2-R10).
    - with `LAB_HOST` unset → the host check is skipped, not run with empty patterns (an empty `-e ""` would match every line).
- [ ] **Step 2:** Run `node --test scripts/test/`. Expected: FAIL (modules missing).
- [ ] **Step 3:** Implement `leak-gate.sh`.
  - It checks with `grep -inEf "$list"`, plus `grep -nwF -e "${LAB_HOST%@*}" -e "${LAB_HOST#*@}"` when `LAB_HOST` is set.
  - Input:
    - `--staged`: `git diff --cached -U0 | grep '^+'`, piped in.
    - Otherwise the files are passed as args.
  - It maps grep's exit codes explicitly (1 → clean) and never lets an error fall through as clean.
- [ ] **Step 4:** Implement `guide-path.mjs` and `sync-labs.mjs`.
  - Regexes:
    - `<!--SRC:([\w.\-/]+)(?: lines=([\d,\-]+))?-->`
    - `<!--CAP:([\w.\-]+)(?: lines=([\d,\-]+))?-->`
  - Scan left to right, pairing each open marker with the next matching close after it.
  - SHA-256 comes from `node:crypto`.
  - In `--check` mode, never write; exit 1 with the errors.
- [ ] **Step 5:** Modify both existing scripts as listed under **Produces**.
- [ ] **Step 6:** Run `node --test scripts/test/`. Expected: all pass.
  - Then `grep -nE 'gen-ai-\$\{|/Users/|/home/' scripts/*.mjs`. Expected: no hits (no hard-coded slug prefixes or absolute paths).
  - Then `scripts/leak-gate.sh --staged` → exit 0.
- [ ] **Step 7:** Commit `feat(memdbg): leak gate, sync-labs, worktree-safe guide scripts`.

### Task 2: Lab environment and capture pipeline

**Files:**
- Create: `labs/memory-debugging/{Dockerfile,Makefile,README.md,captures.tsv,expect.tsv,run-capture.sh,check.sh,remote.sh}`, `captures/00-env.txt`.

**Produces:**
- The image tag `mdg-lab`.
- `./remote.sh [capture-id …]` (needs `LAB_HOST`). In order, it:
  1. rsyncs to `~/mdg-lab/`;
  2. runs `docker build -t mdg-lab .`;
  3. runs `run-capture.sh <ids>` and then `check.sh <ids>` in the container, using the Global Constraints flags plus `$RR_CAPS` from `.rr-caps` (written by Step 5);
  4. runs `chown` in the container. The command is sent in **single quotes**, so the remote host's shell expands the uid and gid before Docker runs: `ssh "$LAB_HOST" 'cd mdg-lab && docker run --rm -v "$PWD":/lab mdg-lab chown -R "$(id -u):$(id -g)" /lab'` (review R2-M5);
  5. rsyncs `captures/` back;
  6. **on the Mac**, runs `scripts/leak-gate.sh labs/memory-debugging/captures/*.txt` and exits non-zero if it does (review R2-H1). The container has no repo scripts, no git and no leak list, so the leak check can't run inside it.
- `run-capture.sh` writes one `# src <file> <sha256>` line per listed source, then `# cmd <command>`, then the combined stdout+stderr. Control bytes are stripped (Review Focus 3). Exit status is ignored; the check decides.
- `check.sh` (in the container) only matches `expect.tsv` and exits 1 on any missing regex. `LAB_HOST` is used only on the Mac and never written to disk.

- [ ] **Step 1:** Write `expect.tsv` for `00-env`:
  - `gcc .*13\.`
  - `clang version 18`
  - `GNU gdb`
  - `valgrind-3\.2`
  - `rr version`
  - `rr-smoke\((perfmon|sys_admin|privileged|none)\)`
- [ ] **Step 2:** Run `./remote.sh 00-env` with no image yet. Expected: FAIL.
- [ ] **Step 3:** Write the Dockerfile.
  - `FROM ubuntu:24.04@sha256:<digest>`. Resolve the digest on the host with `docker buildx imagetools inspect`.
  - Install: `build-essential clang-18 libclang-rt-18-dev gdb valgrind rr heaptrack libc6-dbg binutils file procps`.
  - Make `clang-18` the `clang` alternative.
- [ ] **Step 4:** Write the Makefile (variant table plus `core-%`).
  - `captures.tsv` row `00-env`: the versions of gcc, clang, gdb, valgrind and rr; `ldd --version | head -1`; `uname -m`; `ulimit -c` (expect `0`).
- [ ] **Step 5: rr smoke test (review R1).** `remote.sh --rr-probe` runs `rr record -n /bin/true` three times: with `--cap-add PERFMON`, then `--cap-add SYS_ADMIN`, then `--privileged`.
  - It writes the result to `.rr-caps` in the lab directory, so it syncs into `/lab`. Line 1 is the label (`perfmon`, `sys_admin`, `privileged` or `none`); line 2 is the flags. The file is git-ignored via `$(git rev-parse --git-common-dir)/info/exclude`.
  - The `00-env` row's command itself prints `rr-smoke($(head -1 .rr-caps 2>/dev/null || echo none))`. Re-capturing `00-env` therefore always reproduces the line; nothing is appended by hand (review R2-M6).
  - With `none`, rr captures use the representative fallback.
  - Record the result in the commit body.
- [ ] **Step 6:** Run `./remote.sh 00-env`. Expected: `PASS 00-env`.
- [ ] **Step 7:** Write `README.md`:
  - build the image;
  - `make 06-uaf.asan && ./06-uaf.asan`;
  - `make core-01-null ARGS=field`;
  - why containers can't change `core_pattern`, and why `--ulimit core=0`;
  - each `docker run` flag with a one-line reason.
- [ ] **Step 8:** `scripts/leak-gate.sh --staged`, then commit `feat(memdbg): lab container, Makefile and capture pipeline`.

### Task 3: `pktd.h`, foundation labs and the GDB helper

**Files:**
- Create: `pktd.h`, `f1-layout.c`, `f2-frames.c`, `f3-chunks.c`, `pktd-gdb.py`, `gdb/{f2-frames,f3-chunks,pktd-sessions}.gdb`.
- Add rows to both TSVs.

**Interfaces:** `pktd.h` holds the canon structs (Global Constraints) and the following:
- `SESSION_MAGIC 0x5E55102Bu` and `SESSION_DEAD 0xDEADBEEFu`.
- `extern pthread_mutex_t table_lock, stats_lock`.
- `struct session *session_table[64]`.
- `struct { uint64_t pkts, drops; } stats`.
- `struct session *session_new(uint32_t id, const struct peer *p)`, `void session_get(struct session *)` and `void session_put(struct session *)`, all `static inline`. `session_put` frees at 0.
- `void session_check(struct session *)`. In normal builds it is an empty `static inline`. `table_dump()` calls it on every entry before it touches any field (review R2-M2).
- When `PKTD_REFTRACE` is set:
  - get and put record `{op, refcnt_after, caller (__builtin_return_address(0)), tid}` into a **side table** `static struct ref_ring rings[64]` indexed by `id`. The ring lives outside the struct, so the canon size `_Static_assert` still holds.
  - `session_put` reaching 0 **poisons instead of freeing**: `magic = SESSION_DEAD` and bytes `0x10..0x37` (`created_ns`, `peer`, `name`) set to `0x5A`. `refcnt` and `id` are left intact, and the chunk is quarantined (never freed).
  - `session_check` aborts on `SESSION_DEAD` after printing `session <id>: use after final put` and that id's ring, symbolized with `backtrace_symbols_fd`.
  - `assert(refcnt >= 0)` stays as an extra guard.

- [ ] **Step 1:** Write the expectations.
  - `f1-layout.plain`: `\[heap\]`, `\[stack\]` and `r-xp .*f1-layout`. The lab reads its own `/proc/self/maps` **in-process** with `fopen` and copies it to stdout (review L4).
  - `f2-frames.gdb`: `Stack level 0`, `rip = 0x` and `saved rip`.
  - `f3-chunks.gdb`: `0x0000000000000051` (a 0x48 request rounds to a 0x50 chunk, plus `PREV_INUSE`), and after the free a non-zero mangled `next` word. The guide decodes it as `(pos>>12) ^ next` (safe-linking) next to the `tcache_key` word.
  - The `pktd-sessions.gdb` capture moves to Task 6 (review R2-L7). It needs a binary that fills `session_table`, and the first one is `10-refcount-uaf`. Task 3 writes only `pktd-gdb.py` and `gdb/pktd-sessions.gdb`.
- [ ] **Step 2:** Run `./remote.sh f1-layout.plain f2-frames.gdb f3-chunks.gdb`. Expected: FAIL.
- [ ] **Step 3:** Write the sources.
  - `f2-frames.gdb`: break innermost, then `bt`, `info frame`, `x/4gx $rsp`, `disassemble`.
  - `f3-chunks.c`: three 0x48 mallocs, free the middle one. The `.gdb` does `x/8gx p-16` before and after the free.
  - `pktd-gdb.py`: the `pktd-sessions` command.
- [ ] **Step 4:** Re-run. Expected: all PASS.
- [ ] **Step 5:** Commit `feat(memdbg): pktd header, foundation labs, gdb helper` (with captures).

### Task 4: Ladder labs 01–05

**Files:**
- Create: `01-null.c` … `05-double-free.c` and `gdb/*.gdb`.
- Add TSV rows.

Each lab is ≤ 80 lines, uses `#include "pktd.h"`, and is framed as a `pktd` code path. Run `01-null.core` first: if a GDB-written core does not print `Program terminated with signal` (review R8), change that regex to the real text before writing the rest.

| Capture id | Command | Expected (ERE, all must match) |
|---|---|---|
| `01-null.core` | `make core-01-null ARGS=plain`; gdb `-batch -x gdb/01-null-core.gdb` (`bt`, `frame 1`, `p s`) | `SIGSEGV`; `s = 0x0` |
| `01-null.field.core` | `make core-01-null ARGS=field`; gdb: `bt`, `x/i $pc`, `p $_siginfo._sifields._sigfault.si_addr`, `ptype /o struct session` | `0x18`; `/\* 0x0018 .*peer` |
| `01-null.fnptr.core` | `make core-01-null ARGS=fnptr`; gdb: `bt`, `p $pc`, `x/gx $rsp`, `info symbol *(long*)$rsp`. The NULL handler is called from the non-static `ctrl_dispatch()` | `0x0`; `ctrl_dispatch \+ [0-9]+ in section \.text` (review R2-L2) |
| `02-uninit.memcheck` | `valgrind --track-origins=yes ./02-uninit.plain` | `Conditional jump or move depends on uninitialised value`; `created by a stack allocation` |
| `02-uninit.msan` | `./02-uninit.msan` | `MemorySanitizer: use-of-uninitialized-value` |
| `02-uninit.pattern` | `./02-uninit.pattern` (prints the uninitialised `uint32_t` in hex) | `0xfefefefe` |
| `03-heap-overflow.asan` | `./03-heap-overflow.asan` | `heap-buffer-overflow`; `0 bytes after`; `Shadow bytes around the buggy address` |
| `03-heap-overflow.plain` | `./03-heap-overflow.plain` | `free\(\): invalid pointer`. With size 0, `_int_free`'s `p > -size` check fires before the size check (review R2-M1). The guide explains why the message says "pointer" when the size is what's corrupt. |
| `03-heap-overflow.core` | `make core-03-heap-overflow`; gdb: `frame` of the lab's free caller, `x/4gx b-16` | `0x0000000000000000` (b's size field zeroed by the NUL) |
| `04-stack-smash.plain` | run | `stack smashing detected` |
| `04-stack-smash.asan` | run | `stack-buffer-overflow` |
| `04-stack-smash.core` | `make core-04-stack-smash`; gdb: `bt`, `x/16gx $rsp`, `info symbol` on candidate return addresses | `__stack_chk_fail` |
| `05-double-free.plain` | run | `free\(\): double free detected in tcache 2` |
| `05-double-free.asan` | run | `attempting double-free`; `freed by thread`; `previously allocated by thread` |
| `05-double-free.core` | `make core-05-double-free`; gdb `bt` | `abort`; `free` |
| `t2.build-id` | `make 01-null.stripped; file 01-null.stripped; readelf -n 01-null.stripped \| grep 'Build ID'` | `BuildID`; `stripped` |

- **01-null** takes a variant via argv: `plain`, `field` or `fnptr`. `field` reads `s->peer->addr` with `s == NULL`, so the fault is at `0x18`. `fnptr` calls a NULL handler.
- **03-heap-overflow** (review R3): two 0x48 allocations `a` and `b`, then `strcpy(a, <72-char string>)`. The terminating NUL lands at `a[0x48]`, which is the low byte of b's size field, turning 0x51 into 0x00. Then `free(b)`.

- [ ] **Step 1:** Add all expect rows.
- [ ] **Step 2:** `./remote.sh <ids>`. Expected: FAIL.
- [ ] **Step 3:** Write the labs and `.gdb` files.
- [ ] **Step 4:** Re-run. Expected: all PASS. If a message differs, fix the regex to the real text (never the other way round) and note it in the commit body.
- [ ] **Step 5:** Commit `feat(memdbg): ladder labs 01-05 with captures`.

### Task 5: Ladder labs 06–09

**Files:**
- Create: `06-uaf.c` … `09-deadlock.c`, `gdb/*.gdb`.
- Add TSV rows.

| Capture id | Command | Expected |
|---|---|---|
| `06-uaf.asan` | run | `heap-use-after-free`; `freed by thread T`; `previously allocated by thread T` |
| `06-uaf.memcheck` | `valgrind ./06-uaf.plain` | `Invalid read of size 8`; `inside a block of size .* free'd` |
| `06-uaf.core` | `make core-06-uaf`; gdb: `bt`, `p/x job->buf`, `p/x (unsigned long)job >> 12`, `p (unsigned long)job->buf == ((unsigned long)job >> 12)` | `SIGSEGV`; `= 1$` (review R2-R11). `struct rx_job` is 40 bytes, a size class no other lab allocation uses, and exactly one is allocated. Its tcache bin is therefore empty before the free, and the safe-linked `next` is `(pos>>12) ^ 0`. |
| `07-leak.lsan` | `./07-leak.asan` | `detected memory leaks`; `Direct leak of` |
| `07-leak.memcheck` | `valgrind --leak-check=full ./07-leak.plain` | `definitely lost` |
| `07-leak.massif` | `valgrind --tool=massif --massif-out-file=/tmp/m ./07-leak.plain; ms_print /tmp/m \| head -40` | `Number of snapshots`; `Detailed snapshots` |
| `07-leak.heaptrack` | `heaptrack -o /tmp/h ./07-leak.plain; heaptrack_print "$(ls /tmp/h*.zst \| head -1)" \| head -40` | `peak heap memory consumption`; `leaked` |
| `08-race.tsan` | `./08-race.tsan` | `WARNING: ThreadSanitizer: data race`; `Previous (write|read) of size`; `stats` |
| `08-race.tsan-atomic` | `./08-race.tsan atomic` (uses `atomic_fetch_add`) | `actual=4000000` |
| `08-race.helgrind` | `valgrind --tool=helgrind ./08-race.plain` | `Possible data race` |
| `08-race.drd` | `valgrind --tool=drd ./08-race.plain` | `Conflicting (load|store)` |
| `08-race.lost-updates` | `./08-race.plain` | `expected=4000000 actual=[0-9]+` |
| `09-deadlock.tsan` | `./09-deadlock.tsan inversion` | `lock-order-inversion` |
| `09-deadlock.helgrind` | `valgrind --tool=helgrind ./09-deadlock.plain inversion` | `lock order .* violated` |
| `09-deadlock.gcore` | `./09-deadlock.plain hang &`, sleep 2, `gcore -o cores/09 $!`, kill; gdb on the core: `info threads`, `thread apply all bt`, `p table_lock.__data.__owner`, `p stats_lock.__data.__owner` | `__lll_lock_wait|futex_wait`; `__owner = [1-9]` |

- **06-uaf** frees an `rx_job { struct pktbuf *buf; … }`, then reads `job->buf->len`. tcache's safe-linked `next` overwrites `buf` with `pos>>12`, an unmapped address, so the plain build faults deterministically.
- **08-race**: 4 workers × 1,000,000 `stats.pkts++`. In `atomic` mode the increment is an atomic RMW instead.
- **09-deadlock** has two modes (review H3):
  - `inversion`: thread A takes `table_lock` → `stats_lock` and releases both. A barrier follows, then thread B takes `stats_lock` → `table_lock`. No hang; both lock edges are recorded, so the detectors report.
  - `hang`: the `usleep` interleave that really deadlocks. It is used only for `gcore`.
- **TSan known issue:** if it aborts with `unexpected memory mapping`, run the TSan rows under `setarch -R` and note it in the README.

- [ ] **Step 1:** Add the expect rows.
- [ ] **Step 2:** `./remote.sh <ids>`. Expected: FAIL.
- [ ] **Step 3:** Write the labs.
- [ ] **Step 4:** Re-run. Expected: all PASS (same regex-fix rule as Task 4).
- [ ] **Step 5:** Commit `feat(memdbg): ladder labs 06-09 with captures`.

### Task 6: Boss lab 10, the stripped core, rr, and bridge captures

**Files:**
- Create: `10-refcount-uaf.c`, `gdb/{10-core,10-watch,10-rr,t2-stripped}.gdb`.
- Add TSV rows.

**Scenario (fixed).** Barriers sequence the threads, so the order is deterministic. `handle_timeout`, `worker_main` and `table_dump` are **non-static**, so `-rdynamic` can symbolize them.
1. The control thread creates session 7 with `peer` = `&peers[2]`. Its refcnt is 1, the table's reference.
2. Worker 2 calls `session_get` → 2.
3. Worker 3's error path `handle_timeout()` calls `session_put` **without** a matching get → 1.
4. Worker 2 calls `session_put` → 0 → `free()`.
5. Worker 2 then `malloc`s a `struct pktbuf` and fills **every field** (`len`, `port`, `ts`, then `data` = `"ABCDEFGHIJKLMNOP…"`). This is the same thread, and tcache is per-thread LIFO, so it gets the same chunk. Writing `port` (offset 4) is what makes the pktbuf fill the newest write to `refcnt`.
6. The control thread's `table_dump()` calls `session_check(s)`, which does nothing in this build, then reads `session_table[7]->peer->addr`. `peer` (offset `0x18`, which is `data[8..15]`) now holds `0x504f4e4d4c4b4a49` (ASCII `IJKLMNOP`, little-endian). That address is non-canonical, so the result is a deterministic SIGSEGV. No garbage strings are printed (review R6).

| Capture id | Command | Expected |
|---|---|---|
| `pktd-sessions.gdb` | gdb `./10-refcount-uaf.plain -x gdb/pktd-sessions.gdb`: `source pktd-gdb.py`, break at `control_ready` (the point just after the control thread has created sessions 1–8), `run`, `pktd-sessions`, `kill`, `quit` | `session id=7 refcnt=1 magic=0x5e55102b` |
| `10.plain-x5` | 5× `./10-refcount-uaf.plain; echo "rc=$?"`, then emit `unique=$(… \| sort -u \| wc -l)` | `rc=139`; `unique=1` |
| `10.core` | `make core-10-refcount-uaf`; gdb: `bt`, `frame function table_dump`, `p *s`, `p/x s->magic`, `p *(struct pktbuf *)s`, `pktd-sessions` | `table_dump`; `peer = 0x504f4e4d4c4b4a49`; `len = ` |
| `10.asan` | `./10-refcount-uaf.asan` | `heap-use-after-free`; `session_put`; `handle_timeout|worker_main` |
| `10.watch` | gdb: break after `session_new(7`, `watch -l s->refcnt`, `commands; bt 3; continue; end`, run. Hits in order: get 1→2 (worker 2), put 2→1 (`handle_timeout`), put 1→0 (worker 2), `tcache_put` in `free`, then the pktbuf `port` store (worker 2) (review R2-L3) | `Hardware watchpoint`; `Old value = 2`; `handle_timeout` |
| `10.rr` | `rr record ./10-refcount-uaf.plain`, then `rr replay -x gdb/10-rr.gdb </dev/null`. The script: `continue` to SIGSEGV, `frame function table_dump`, `watch -l s->refcnt`, then 4× (`reverse-continue`, `bt 2`), and it ends with `quit` (review R2-R12). Stops, newest first: worker 2's pktbuf fill → `tcache_put` in `free` → worker 2's `session_put` → `handle_timeout`'s `session_put` (review H4) | `tcache_put|_int_free`; `handle_timeout` (or the fallback) |
| `10.reftrace` | `./10-refcount-uaf.reftrace` | `session 7: use after final put`; `put .*handle_timeout`; `put .*worker_main` |
| `t2.stripped-core` | `make core-10-refcount-uaf V=stripped`. Then `mkdir -p cores/nodebug && cp 10-refcount-uaf.stripped cores/nodebug/`, so the debuglink can't find `.debug` next to the binary (review R2-H2). gdb `-ex 'set debug-file-directory /nonexistent' cores/nodebug/10-refcount-uaf.stripped cores/10-refcount-uaf.stripped.core`: `bt` and `info threads` (no symbols), then `symbol-file 10-refcount-uaf.debug`, `bt` and `thread apply all bt 1`, `quit` | `\?\?`; `table_dump`; `worker_main` |
| `x1.stress` | 200× `./08-race.plain`, then `distinct=$(grep -o 'actual=[0-9]*' \| sort -u \| wc -l)` | `distinct=([2-9]|[1-9][0-9]+)` |
| `x1.rr-chaos` | 3× `rr record --chaos ./08-race.plain`, then one `rr replay -a` showing the same `actual=` | `actual=[0-9]+` (or the fallback) |

- [ ] **Step 1:** Add the expect rows.
- [ ] **Step 2:** `./remote.sh <ids>`. Expected: FAIL.
- [ ] **Step 3:** Write the lab and `.gdb` files.
- [ ] **Step 4:** Re-run. Expected: all PASS, including `unique=1`.
- [ ] **Step 5:** Commit `feat(memdbg): boss refcount UAF lab, stripped core, rr and stress captures`.

### Task 7: Guide shell and components

**Files:**
- Create: `memory-debugging-guide.html`.
- Modify: `$(git rev-parse --git-common-dir)/info/exclude` — add `.build/` if it is absent.

**Produces:**
- **Markers** `<!--PART:part0-->…<!--/PART:part0-->` through `part3a`, `part3b` and `ref`, in table order.
- **Sidebar:** 33 targets in the section-id groups, labelled with the spec §6 titles. "Bridge" is its own group.
- **CSS** (tokens only, plus a `body.light` variant):
  - `.term` (mono, `overflow-x:auto`) and `.term.representative` (dashed border plus a `::before` label);
  - `pre.excerpt` and `ol.callouts`;
  - `.ab` grid with `.ab-a` ("It reproduces") and `.ab-b` ("Only a core") — two columns at ≥ 768px, stacked below;
  - `details.deep` (summary "Deep dive");
  - `.hook` (label "→ This is why it works");
  - `details.src`;
  - `.mem-table`.
- **JS:** `.copy` buttons call `navigator.clipboard.writeText(btn.parentElement.querySelector('code').textContent)`.

- [ ] **Step 1:** `cp gen-ai-harness-engineering-guide.html memory-debugging-guide.html`. Strip all sections and add the 6 marker pairs. Remove the CSS for `q-tag`, `hype-tag`, `np` and `skill-box`.
- [ ] **Step 2:** Set the title, brand, tagline ("GDB, sanitizers, Valgrind & rr — from the memory model up") and accent.
- [ ] **Step 3:** Add the components and the JS, and rebuild the sidebar.
- [ ] **Step 4:** Write a scratch `part0` that exercises every component, including a `.term` with a 200-char line, and placeholder fragments `<!-- placeholder: <part> -->` for the other five.
  - Run `node scripts/assemble-guide.mjs memory-debugging --out <scratchpad>/shell.html`.
  - Take headless Chrome screenshots at 1280px and 390px, dark and light. Move `active` to the target section and set `class="light"` on `body`; never call `go()`.
  - Expected: no page-level horizontal scroll at 390px; panels stacked; the representative label visible.
- [ ] **Step 5:** Run the `--out` assemble twice into two scratch files. Expected: both exit 0 and the `shasum`s are equal (review M7).
- [ ] **Step 6:** `scripts/leak-gate.sh --staged`, then commit `feat(memdbg): guide shell, components, nav`. The scratch fragments are not committed.

### Tasks 8–12: Content fragments (parallel once Tasks 1–7 are done)

**What each task touches:**
- Each task writes **only** its own `.build/memory-debugging/<fragment>.html`.
- No task touches the HTML, runs scripts, or commits.

**Content rules:**
- Every terminal block is a CAP marker for an existing capture.
- Every lab shown gets a full `details.src` block in the chapter where it first appears.
- Inline code from a lab is always a `SRC … lines=` excerpt, never hand-copied (review L7).
- Diagram markers: `<figure class="diagram"><!--DIAGRAM:<id>--><!--/DIAGRAM:<id>--><figcaption class="ann">…</figcaption></figure>`.

**Chapter template:**
- `<section id class="section">`, `<h1>N. Title</h1>`, `<p class="lede">What you'll learn: …</p>`;
- 3–6 `.card` subsections, then ≥ 1 `details.deep`, then `.see-also`.
- **Foundation chapters** end with a `.hook` naming the tool and the Part II chapter it explains.
- **Tool chapters** open with a back-link to that Part I chapter.
- **Ladder chapters** have exactly one `.ab`, with both panels, and a "Symptom → class" line that matches `ref-decision`.

**Self-check per fragment:**
```bash
g=.build/memory-debugging/<fragment>.html
echo "div $(grep -o '<div' $g|wc -l)/$(grep -o '</div>' $g|wc -l)"
for id in <ids>; do grep -q "id=\"$id\" class=\"section" $g && echo "ok $id" || echo "MISSING $id"; done
for c in $(grep -o '<!--CAP:[^ >]*' $g | cut -d: -f2 | sed 's/--$//'); do test -f labs/memory-debugging/captures/$c.txt || echo "NO CAPTURE $c"; done
for s in $(grep -o '<!--SRC:[^ >]*' $g | cut -d: -f2 | sed 's/--$//'); do test -f labs/memory-debugging/$s || echo "NO SOURCE $s"; done
scripts/leak-gate.sh $g; echo "leak-gate exit $?"
```
Expected: balanced, `ok` for every id, no `NO …` lines, and `leak-gate exit 0`.

#### Task 8: part0 + part1 (Welcome, Map, Ch 1–6)
- **welcome:**
  - who it's for (spec §2) and how to read it;
  - running the labs, with diagram `capture-pipeline` (review M8);
  - the toolchain (CAP `00-env`);
  - a note that addresses and PIDs will differ on your machine;
  - SRC for `Dockerfile`, `Makefile` and `pktd.h`, and a pointer to `ref-labkit`.
- **map:**
  - a `.tbl` of bug class × tool × path;
  - meet `pktd`, with the canon table;
  - diagram `path-ab-flow`.
- **f1-layout:** CAP `f1-layout.plain`. Hook → `t1` and `t3` (`info proc mappings`).
- **f2-frames:**
  - a stack-frame `.mem-table`;
  - SysV argument registers (rdi, rsi, rdx, rcx, r8, r9), callee-saved registers, red zone, frame pointer, canary slot;
  - CAP `f2-frames.gdb`;
  - hook → `b4`.
- **f3-allocator:**
  - a chunk-header `.mem-table`: prev_size, size with the A/M/P bits, then for free chunks `next`/fd, `tcache_key`/bk;
  - **safe-linking**: tcache and fastbin `next` stored as `(pos>>12) ^ ptr` since glibc 2.32, with the decode shown against CAP `f3-chunks.gdb` (review M5);
  - bins and tcache (7 per size), per-thread arenas, top chunk;
  - a Deep dive decoding common glibc abort strings;
  - hook → `t3`, `b3`, `b5` and `b6`.
- **f4-instrumentation:**
  - diagram `instrumentation-models`;
  - a shadow-mapping `.mem-table`, redzones, quarantine;
  - DBT, rr recording, and the one-line perf-counter note;
  - a slowdown/compatibility `.tbl`;
  - hook → `t4`–`t7`.
- **f5-memory-model:** diagram `happens-before`; why `volatile` doesn't help. Hook → `t5`.
- **f6-atomics-locks:**
  - atomic RMW, memory orders, mutex `__owner`;
  - diagram `lock-cycle`;
  - refcount patterns;
  - hook → `t5`, `t6`, `b9` and `b10`.

#### Task 9: part2 (Ch 7–13)
- **t1-gdb-live:**
  - breakpoints, `watch` and `watch -l` (hardware vs. software), `thread apply all bt`, `x`, `info frame`, `scheduler-locking`;
  - reuses the `f2` and `10.watch` captures.
- **t2-cores-symbols:**
  - `ulimit -c`, `core_pattern` (a generic example only), `coredumpctl` (representative-labelled), `gcore`;
  - build-id (CAP `t2.build-id`) and split debuginfo;
  - **stripped-core forensics: CAP `t2.stripped-core`** (review M10);
  - debuginfod;
  - why the labs use GDB-written cores plus `--ulimit core=0`.
- **t3-gdb-core:** what a core holds; SRC `pktd-gdb.py`; CAP `pktd-sessions.gdb`.
- **t4-asan:**
  - ASan, UBSan, LSan, MSan, with HWASan in a Deep dive;
  - CAP `06-uaf.asan` with callouts on the three stacks;
  - an `ASAN_OPTIONS` table.
- **t5-tsan:**
  - CAP `08-race.tsan`, then `08-race.tsan-atomic`;
  - **how TSan treats atomics**: C11/`__atomic` builtins are understood; hand-rolled `volatile` flags or inline-asm locks produce false positives (review L6);
  - CAP `09-deadlock.tsan`;
  - suppressions and the `setarch` note.
- **t6-valgrind:**
  - Memcheck, Helgrind, DRD (CAP `08-race.drd`) and Massif; when Valgrind beats sanitizers;
  - a Deep dive on `MALLOC_CHECK_=3`, `MALLOC_PERTURB_`, `mtrace` and Electric Fence.
- **t7-rr:** record and replay, `reverse-continue`, `watch` + reverse, `--chaos`, using CAP `10.rr` and `x1.rr-chaos` (or the fallback).

#### Task 10: part3a (Ch 14–18)
One section per rung `b1`–`b5`, with the Task 4 captures in `.ab` panels.
- **b1:**
  - all three NULL variants;
  - `ptype /o` mapping `0x18` to `peer`;
  - recovering the caller from `[$rsp]`.
- **b2:** explains `0xfefefefe` (GCC) vs `0xaaaaaaaa` (Clang).
- **b3:**
  - a shadow-byte `.mem-table` read from the CAP;
  - the NUL off-by-one zeroing b's size byte.
- **b4:** a manual unwind, plus `_FORTIFY_SOURCE` (Deep dive).
- **b5:** the tcache key.

#### Task 11: part3b (Ch 19–24)
- **b6–b9:** the Task 5 captures.
  - **b6** decodes the safe-linked `job->buf` value.
  - **b7** covers fragmentation vs. a leak, with `malloc_info` in a Deep dive.
  - **b9:**
    - why the detectors use the `inversion` run while the core uses the `hang` run;
    - mapping `__owner` → LWP → `info threads`.
- **b10-refcount-uaf:**
  - the scenario, with diagram `boss-timeline`;
  - four `.card`s:
    1. forensics (`10.core`): `peer` = ASCII `IJKLMNOP`, which gives away the pktbuf. CAP `10.plain-x5` proves the crash is identical on every run (review R2-L1);
    2. ASan (`10.asan`);
    3. watchpoint and rr (`10.watch`, `10.rr`), explaining the allocator's own stop;
    4. design (`10.reftrace` plus a SRC excerpt of the `PKTD_REFTRACE` block);
  - a design checklist, repeated in the cheat sheet.
- **x1-core-to-repro:**
  - core → hypothesis → harness;
  - jitter, `sched_yield`, `MALLOC_PERTURB_`;
  - CAP `x1.stress` and `x1.rr-chaos`;
  - diagram `core-to-repro`.

#### Task 12: ref
- **ref-decision:** symptom → class → first tool for A and for B, ≥ 10 rows. Each ladder chapter's "Symptom → class" line appears verbatim.
- **ref-cheat:** the commands, plus the b10 checklist.
- **ref-gdb-lldb:** ≥ 20 pairs, plus a macOS note.
- **ref-flags:** the variant table plus a compatibility matrix.
- **ref-labkit:** a SRC block for every lab file not shown in full elsewhere: `README.md`, `run-capture.sh`, `check.sh`, `remote.sh`, `captures.tsv`, `expect.tsv` and every `gdb/*.gdb` not embedded in a chapter.
- **ref-glossary:** ≥ 40 terms.
- **ref-reading:**
  - primary docs, fetched and dated "as of Nov 2026";
  - `perf c2c` as a one-line see-also.

### Task 13: Integrate, diagrams, gates

**Files:**
- Create: `diagrams/memory-debugging/_registry.mjs`, plus `{capture-pipeline,path-ab-flow,instrumentation-models,happens-before,lock-cycle,boss-timeline,core-to-repro}.mmd`.
- Modify: `README.md`, `cspell.json`.

- [ ] **Step 1:** Write the 7 `.mmd` files and the registry (`kind: 'mermaid'`). Then `grep -c '<!--DIAGRAM:' .build/memory-debugging/*.html | awk -F: '{s+=$2} END {print s}'`. Expected: `7`, with each registry id present.
- [ ] **Step 2:** Run in order:
  1. `node scripts/assemble-guide.mjs memory-debugging` (**final assemble**);
  2. `node scripts/sync-labs.mjs memory-debugging --write`;
  3. `node scripts/build-diagrams.mjs memory-debugging`.

  Expected: `injected 7 diagram(s)`.
- [ ] **Step 3: Gates.** All must pass.
  - **Nav parity:** the set of `go('x')` targets equals the set of section ids; both are 33.
  - **Div balance.**
  - **Lab sync:** `node scripts/sync-labs.mjs memory-debugging --check` exits 0.
  - **Every tracked lab file is embedded:**
    ```bash
    cd labs/memory-debugging && git ls-files | grep -v '^captures/' | while read f; do grep -q "SRC:$f\( lines=[0-9,-]*\)\?-->" ../../memory-debugging-guide.html || echo "UNEMBEDDED $f"; done
    ```
    Prints nothing (review M9).
  - **Every capture is used:** for each `captures/<id>.txt`, `grep -qE "CAP:<id>( lines=[0-9,-]*)?-->"` succeeds, with `<id>`'s dots escaped. This is an exact match, so `08-race.tsan-atomic` can't satisfy `08-race.tsan` (review R2-L6).
  - **Structure:** `class="hook"` ≥ 6; `class="ab"` = 10; `ab-a` = `ab-b` = 10.
  - **Brief mentions:** `HWASan`, `MALLOC_CHECK_`, `MALLOC_PERTURB_`, `mtrace`, `Electric Fence`, `_FORTIFY_SOURCE`, `debuginfod` and `safe-linking` each ≥ 1.
  - **Leak:** `git ls-files memory-debugging-guide.html labs/memory-debugging diagrams/memory-debugging README.md docs/superpowers/specs/2026-11-17-memory-debugging-guide-design.md docs/superpowers/plans/2026-11-17-memory-debugging-guide.md | xargs scripts/leak-gate.sh` exits 0. It passes files only, never directories, whatever the shell (review R2-M4).
  - **cspell:** `npx -y cspell@8.14.4 --no-progress memory-debugging-guide.html labs/memory-debugging/README.md`. Add genuine terms to `cspell.json`.
  - **Render:** screenshots of `welcome`, `f3-allocator`, `b10-refcount-uaf` and `ref-decision` at 1280px dark and light and at 390px. Legible, with no page-level horizontal scroll.
- [ ] **Step 4:** Add a "Systems Debugging" section to the README, between Data Center & EVPN and Gen AI.
- [ ] **Step 5:** `scripts/leak-gate.sh --staged`, then commit `docs(memdbg): assemble guide, diagrams, gates`.

### Task 14: Super-review and fix

- [ ] **Step 1:** Run `super-review` on `memory-debugging-guide.html`.
  - It's a large file, so use split agents.
  - Give them the canon and spec §6.
  - Ask them to check every glibc/ASan/TSan/rr claim against the captures.
  - Save as `REVIEW-memory-debugging-guide-R1.md` at the repo root. It stays **local**: `REVIEW-*.md` is git-ignored by repo convention (review M11), so never `git add -f` it.
- [ ] **Step 2:** Apply fixes HIGH → MED → LOW.
  - A fix that edits a lab means re-running its captures and `sync-labs --write`.
  - Then re-run the Task 13 gates.
- [ ] **Step 3:** `scripts/leak-gate.sh --staged`, then commit `fix(memdbg): apply super-review R1`. The commit body summarises the review's findings.
