# Design Spec — Gen AI for Developers, Part 4: Harness, Loop & Graph Engineering

**Status:** v3 — approved by user to proceed; super-review R1 fixes applied (see `REVIEW-2026-10-27-harness-engineering-guide-design-R1.md`)
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-10-27
**Part of series:** Part 4 of "Gen AI for Developers"
**Prerequisites:** Part 1 (`gen-ai-zero-to-hero-guide.html`) and Part 2 (`gen-ai-hero-to-architect-guide.html`). Part 3 (`gen-ai-architect-to-leader-guide.html`) is optional — cross-linked, not assumed.
**Output artifact:** `gen-ai-harness-engineering-guide.html` *(name is Claude's call — see §12)*

---

## 1. Purpose (the why)

During 2026 a set of terms took over agentic-AI discussion — **harness engineering** (popularised ~Feb 2026), **loop engineering** (~Jun 2026), and **graph engineering** (the *practice* of graph-shaped agent workflows is older — LangChain frames it as ~3 years old — but the label spread in 2026) — plus a long tail of hype labels. They are loosely defined, overlap heavily, and are mostly explained in vendor blogs.

This guide exists so the author (and teammates they share it with) can:
1. **Understand** these ideas from first principles — what problem each solves, not just what it's called.
2. **Later use them** to upgrade their existing Claude Code skills/agents or design new ones.

It is a **learning guide, not a cookbook**: every concept should come out as a mental model the reader can hold up against a skill and ask "what would this change?". Actually rewriting skills is explicitly a later, separate activity.

## 2. Audience

- **Primary:** the author — an experienced engineer who already builds approval-gated skills, worker subagents, and AI-driven test generation.
- **Secondary:** teammates the guide is shared with.
- Assumes Part 2 fluency (agent loop basics, context window, tools/permissions, skills, MCP). Does **not** re-teach those; links back.
- Tone: simplify. Plain language, short definitions first, jargon defined on first use. Concise, technical, no fluff.

## 3. Goals / non-goals / anti-goals

**Goals**
- A reader can define each term in one sentence and explain *what problem it solves* and *how it relates to the others*.
- A reader can look at any skill/agent and locate its harness, loop, graph, checker, state, and human gates.
- A reader finishes with a concrete **skill audit checklist** they can apply later.

**Non-goals**
- Not a re-teach of Part 2 (agent loop basics, context/tokens, tool permissions, multi-agent intro). Short recaps + links only.
- No framework comparisons (LangGraph vs CrewAI vs …) and no agent-to-agent protocol coverage (A2A etc.) — they go stale and don't help skill design.
- No spec/intent/environment "engineering" chapters — no real substance yet; mentioned only in the hype decoder.
- Not about building LLM-backed *products* (RAG apps etc.).
- Does not rewrite any actual skill.

**Anti-goals (failure even if it "works")**
- Reads as a vendor-blog glossary: names and buzzwords without the underlying *why*.
- Presents contested 2026 terminology as settled fact.
- Each term taught in isolation, so the reader can't see that loop and graph are parts of the harness.
- Abstract-only — no tie back to skills/agents the reader actually runs.

## 4. Positioning vs. Parts 1–3

| | Part 1 | Part 2 | Part 3 | **Part 4 (this)** |
|---|---|---|---|---|
| Question | What is this? | How do I work with agents? | How does my org adopt it? | **How do I *design* the system around the model?** |
| Altitude | Concepts | Individual practice | Org practice | **System design of agents/skills** |
| Spine | Topic progression | Worked example (`gitlog-report`) | Maturity model + Meridian | **"Agent = Model + Harness" + the fictional `netpush` pipeline as running example** |

Part 4 is a *depth* axis off Part 2, not a successor to Part 3. Overlap to manage explicitly: Part 2 Ch 1 (loop), Ch 2 (context), Ch 3 (tools), Ch 8 (TDD), Ch 10 (safety, review & verification — anchor `ch10-review`; Ch 7 here links to whichever Part 2 anchor actually covers verification), Ch 11 (multi-agent). Each overlapping chapter here opens with a one-paragraph recap + link, then goes deeper on the design angle.

## 5. The conceptual spine (decided in conversation)

> **Agent = Model + Harness.** The harness is everything that isn't the model. It answers six questions:
> 1. **What does it see?** — context
> 2. **What can it do?** — tools, permissions
> 3. **What does it remember?** — state outside the context window
> 4. **How is it checked?** — verification / evals
> 5. **How does it repeat?** — the loop
> 6. **How do the pieces connect?** — the graph (including **human nodes** — approval gates are graph nodes, not a separate layer)
>
> …plus an **outer loop** that improves the harness itself.
>
> These six questions are the single taxonomy used everywhere: chapter grouping, failure-map layers (Ch 13, plus "model" as the non-harness layer), the `netpush` dissection (Ch 16), and the audit rubric (Ch 17). Any change to the list must be propagated to all four.

Key framing: **loop engineering and graph engineering are not new eras on top of harness engineering — they are components inside it** (loop = control flow over time; graph = control flow across components). The guide presents the popular "prompt → context → harness → loop/graph" lineage, then shows why the nested view is more useful.

## 6. Framing rules (apply throughout)

1. **First principles first.** Every chapter: plain one-line definition → the problem it solves (what breaks without it) → mental model/diagram → running-example illustration → **"What this means for your skill"** box → see-also links.
2. **Fully generic — no employer, product, or internal-tool names anywhere** (the repo is public; decided by the user). The running example is a **fictional** set of agent tooling, in the spirit of Part 3's Meridian, labeled illustrative:
   - **`netpush`** — a multi-agent pipeline that pushes config changes to a fleet of (generic, vendor-neutral) routers: *preflight → generate → dry-run → deploy → verify* worker subagents, coordinated by an orchestrator, with a `plan.yaml` as shared state.
   - **`gen-parser`** — an approval-gated skill: propose a schema, human approves, then generate code + tests.
   - **`crash-doctor`** — a debugging skill that appends to a `lessons.md` after every session.
   - **`triage`** — a long-running multi-phase skill that resumes from a state file after context compaction.
   Concrete anchors to illustrate (generic lessons, invented details):
   - deploy step: "read the *full* commit output — a naive `grep error` misses the real failure string" → code-vs-model + checker.
   - dry-run step: "parse-passing is not commit-passing" → limits of a checker.
   - verify step: "assert on state values, never on exit code" → checker design.
   - deploy step: stop on first failure, don't blind-rollback peers → graph edge semantics.
   - `gen-parser` propose-then-generate → humans as graph nodes.
   - `triage` resume → state outside the context window.
   - `crash-doctor` `lessons.md` → primitive self-improving harness.
   Use vendor-neutral CLI/config snippets (or clearly invented syntax); no real vendor, company, or hostname. **Do not read or mine the author's private skill/agent files** (`~/.claude/**`) for examples — invent from the anchors above.
6. **Model names:** use capability language ("a frontier model", "a small fast model"). If a specific model must be named, date it "as of Oct 2026". Do not copy the older model names used in Parts 1–3.

### 6.1 `netpush` canon (single source of truth for the running example)

All chapters must agree with this sheet; the plan may extend it but not contradict it.

| Aspect | Canon |
|---|---|
| Purpose | Push a config change to a small fleet of routers (fictional "Lab A", 6 routers: 2 core, 4 edge) |
| Graph | orchestrator → *preflight* (per router, parallel, read-only) → *generate* (once, central) → *dry-run* (per router, parallel) → **human approval gate** → *deploy* (serial, core first) → *verify* (per router) |
| Shared state | `plan.yaml` (intent + per-router stages) and `reports/<stage>-<router>.json`; the orchestrator resumes from these files |
| Code vs model | Model: generate config from intent; in v1 also summarise the generated config for the approver (v1 computes no diff — gap 4; v2 adds a code-computed diff that the model summarises). Code: parsing CLI output, success/fail matching, ordering, stop rules |
| Checkers | dry-run = syntax/staging only ("parse-passing is not commit-passing"); verify = assert on observed state values, never exit codes |
| Loop rules | generate ↔ dry-run retry max 3, then escalate to human; deploy stops on first router failure, never blind-rolls back peers |
| Budgets | per-run wall-clock and token budget; exceeded → stop and report, never silent retry |
| **Deliberate gaps** (planted early, revealed in Ch 16) | (1) deploy originally matched `grep -i error` and missed a real abort string; (2) verify checker editable by the generate agent (reward-hacking hole); (3) no idempotency marker, so a resumed run could re-deploy a router; (4) approval gate shows raw config, not a diff — rubber-stamp risk |
| Other fictional skills | `gen-parser` (propose → approve → generate; audited in Ch 17), `crash-doctor` (`lessons.md` outer loop, Ch 14), `triage` (state-file resume, Ch 6) |
3. **Claude Code is the concrete surface** for examples (skills, subagents, hooks, `/loop`, scheduled routines); principles stated tool-agnostically with a one-line note where another tool differs. *(Claude's call.)*
4. **Volatile facts are hedged and dated.** Term origins, who coined what, and paper claims carry "as of Oct 2026" and are attributed. Where sources disagree (e.g. who coined "harness engineering"), say so.
5. **Every named "___ engineering" term** (prompt, context, harness, loop, graph, and the hype-decoder terms) gets a "solid vs. hype" marker consistent with the hype decoder.

## 7. Structure — chapter outline

**Front matter**
- **Welcome** — who it's for, prerequisites (Parts 1–2; Part 3 optional), the promise (understand now, upgrade skills later).
- **The Map** — Agent = Model + Harness; the six questions; the outer loop; containment diagram (the harness contains context — which contains the prompt — plus tools, state, checker, loop, and graph); 2026 term timeline; meet the running example.

**Part 1 · First Principles**
1. **The model is not the agent** — why raw models fail as agents; what a harness is; the prompt → context → harness lineage and why the nested view wins.
2. **Code vs. model: where each decision belongs** — the determinism dial; what must be code (parsing, asserting, committing), what should be the model (judgement, synthesis); cost of getting it wrong both ways. *(Flagship chapter.)*
3. **Anatomy of a harness** — the six questions as a component map; a quick tour of how real coding agents implement each (cite the 11-system source study, hedged).

**Part 2 · Harness Engineering**
4. **What it sees** — context as a harness layer: progressive disclosure, skills as on-demand context, what to keep out. Recap + link Part 2 Ch 2.
5. **What it can do** — tools and permissions as harness design: tool surface shape, blast radius, guarded actions on real devices (commit-with-auto-rollback pattern). Recap + link Part 2 Ch 3.
6. **State outside the window** — files as memory, checkpoints, plan/state files, resuming after compaction or crash, idempotent steps.
7. **The checker comes first** — a loop is only as good as its judge: deterministic checks, dry-runs, state assertions, LLM-as-judge and its traps, reward hacking, putting the checker outside what the agent can edit.

**Part 3 · Loop Engineering**
8. **Anatomy of an engineered loop** — goal → act → observe → adjust; who prompts the agent when you don't; retry policy; exit conditions; the "three loops" framing (agent / developer / external feedback) — attribute only if a primary source is found, otherwise present unattributed.
9. **Budgets & stopping rules** *(short)* — token, time, iteration, and cost budgets as stop conditions; failing loud vs. looping forever.
10. **Background & scheduled loops** *(short)* — loops nobody watches: cron/scheduled routines, `/loop`, CI-triggered agents; what extra harness they need (notifications, kill switches, idempotency). Tool-specific commands verified against current docs at build time and hedged "as of Oct 2026".

**Part 4 · Graph Engineering**
11. **From loop to graph** — nodes (code, LLM call, agent, human), edges (routing, conditions), shared/typed state, fan-out/fan-in, orchestrator–worker, stop-on-first-failure. Contrast with Part 2 Ch 11. Running example drawn as a graph.
12. **Humans as nodes** *(short)* — where approval gates belong, propose-then-generate, what the human must see to approve well, gates that become rubber stamps.

**Part 5 · Operating & Evolving**
13. **The failure map** — symptom-indexed: "it did X" → which layer broke (model, or one of the six harness questions) → what to change. Table + decision flow. Recap-and-link Part 2 §1.8 (bad loop), §1.10 (failure modes), and `ch9-debug` rather than re-teaching them.
14. **Self-improving harnesses** — traces in, harness edits out; lessons files; eval-gated harness changes; keep evaluator and permissions outside the evolving loop; current research (hedged).
15. **Hype decoder** *(short)* — one-page table: term → plain meaning → solid / partly real / rebranding / no substance yet → where it lives in the six-question map.

**Part 6 · Putting It to Work**
16. **Dissecting `netpush`** — full walk-through of the running example through every lens (six questions, loop, graph, checker, state, gates, failure map) — what's already strong, where the gaps are.
17. **The skill audit checklist** — the reusable rubric: score any skill/agent per harness question, with a worked audit of the fictional `gen-parser` skill as demonstration (findings only — no rewrite).

**Reference**
- Cheat sheet (one-screen: spine, six questions, code-vs-model rules, loop stop rules, graph patterns).
- Decision frameworks (code or model? loop or graph? where to put the human?).
- Glossary A–Z.
- Further reading — labeled, dated, hedged links (seed list in §11).

Target scale: 17 chapters across 6 Parts + front matter + reference — **smaller than Part 2**, comparable to Part 3.

## 8. Visual style

- Exact match to the series chrome (sidebar nav grouped by Part, dark/light toggle, same tokens, fonts, and component vocabulary: info boxes, comparison grids, step blocks, tables).
- A distinguishable Part-4 accent color *(chosen during build)*.
- Real diagrams via the existing Mermaid → inline SVG pipeline (`diagrams/<slug>/*.mmd` + a per-guide `diagrams/<slug>/_registry.mjs`, built by `scripts/build-diagrams.mjs` with helpers in `scripts/lib/`, CSS-variable theming). Minimum set: the Map nesting diagram, term timeline, code-vs-model dial, harness component map, engineered loop, `netpush` as a graph, failure-map decision flow, self-improving outer loop.
- Series cross-links: welcome note + reference card linking Parts 1–3; Parts 1–3 gain a forward link to Part 4 on completion; `README.md` Gen AI table gets a row.

## 9. Constraints

- Single self-contained HTML; no runtime deps; works offline (per repo `CLAUDE.md`).
- Built with the `guide-builder` skill.
- cspell clean before done.
- **Leak gate (hard, before every commit):** `grep -inEf .git/info/leak-terms <changed files>` must return nothing in the guide, diagrams, README, spec, plan, and review files. Any hit blocks the commit. The term list lives in `.git/info/leak-terms` (local, never committed) precisely so the forbidden terms are not themselves published.
- Review rounds captured as `REVIEW-gen-ai-harness-engineering-guide-R<n>.md`.

## 10. Success criteria

- After reading, the author can (a) define harness/loop/graph engineering in one sentence each, (b) explain why loop and graph live inside the harness, (c) take any of their own skills and produce a gap list using the Ch 17 checklist without re-reading the guide.
- Every chapter except Ch 15 (hype decoder) has a running-example tie-in and a "What this means for your skill" box.
- Leak gate (§9) returns zero hits on every committed file.
- The six-question taxonomy is identical in §5, Ch 3, Ch 13, Ch 16, Ch 17, and the cheat sheet.
- No contested claim stated as fact; every term origin dated and attributed.
- Consistent with Parts 1–3 in voice, components, and terminology; cross-linked both ways.

## 11. Research seed (as of 2026-10-27)

Solid: OpenAI Codex harness post (Feb 2026); LangChain "Agent = Model + Harness"; arXiv 2609.00006 (11-system harness source study), 2604.25850 (Agentic Harness Engineering), 2608.25593 (JIT-Agent), 2608.21884 (Loop Engineering), 2603.22386 (workflow-graph survey); Lilian Weng, "Harness Engineering for Self-Improvement" (Jul 2026); IBM loop-engineering explainer; LangChain "3 Years of Graph Engineering"; Palo IT "Beyond the Prompt…" (loop/graph as harness components); dbreunig "State of Context Engineering" (Oct 2026).
Vendor/secondary (use for color, not authority): Atlan, Faros, Lyzr, MindStudio, Requesty, Data Science Dojo, codecentric, ADTmag.
These were gathered from search-result snippets. **The builder must fetch each source (arXiv abstract pages included) before citing it**, and drop any that don't resolve or don't say what is claimed. Add newer sources found during the build.

## 12. Calls Claude made (review these)

- File name `gen-ai-harness-engineering-guide.html` and title "Harness, Loop & Graph Engineering" (series naming "X to Y" not used, since Part 4 is a depth axis, not a next rung).
- Claude Code as the concrete surface; other tools only as one-line notes.
- Chapter grouping into 6 Parts and the 17-chapter scope.
- Six questions (state promoted to its own question; humans as graph nodes) — from review R1.
- `netpush` canon details (§6.1), including the four deliberate gaps.
- Fictional names `netpush`, `gen-parser`, `crash-doctor`, `triage` for the running example.

## 13. Left to the builder

Exact accent color; diagram visual details; per-chapter section breakdown; exercise/"try it" content; build order (detailed in the implementation plan).
