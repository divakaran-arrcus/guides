# Harness, Loop & Graph Engineering Guide (Part 4) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Load the `guide-builder` skill for the repo's HTML component vocabulary.

**Goal:** Build `gen-ai-harness-engineering-guide.html`, "Gen AI for Developers: Harness, Loop & Graph Engineering" (Part 4). It is a first-principles learning guide with 17 chapters in 6 Parts plus front matter and reference pages, built around a fictional `netpush` running example.

**Architecture:** One self-contained HTML file cloned from Part 3's chrome (sidebar SPA: every `<section class="section">` is hidden, and `go(id)` shows one). Chapter content is written as **per-Part HTML fragments** in a git-ignored `.build/harness-engineering/` directory, so Parts can be written in parallel without edit conflicts. An assembly step splices them into the shell between markers. Diagrams use the existing Mermaid → inline-SVG pipeline.

**Tech Stack:** HTML5, embedded CSS, vanilla JS; Node + `npx @mermaid-js/mermaid-cli@11.4.2` (pinned in `scripts/build-diagrams.mjs`); system Chrome; `npx -y cspell@8.14.4`.

**Spec:** `docs/superpowers/specs/2026-10-27-harness-engineering-guide-design.md`. Read §5 (six questions), §6 (framing rules), §6.1 (`netpush` canon) and §7 (outline) before any content task.

## Global Constraints

- Single self-contained HTML. No external runtime JS/CSS except the Google Fonts link already in the Part 3 chrome. Works offline.
- **No employer, product or internal-tool names.** Every task's commit runs the leak gate on *added lines*: `git diff --cached -U0 | grep '^+' | grep -inEf .git/info/leak-terms` must print nothing. Do not read `~/.claude/**` skill or agent files for examples.
- Six-question taxonomy, verbatim and in this order everywhere: **What does it see? · What can it do? · What does it remember? · How is it checked? · How does it repeat? · How do the pieces connect?**
- `netpush` facts come only from spec §6.1. Fictional names only: `netpush`, `gen-parser`, `crash-doctor`, `triage`, "Lab A".
- Model names: capability language. If a model must be named, date it "as of Oct 2026". Don't copy Parts 1–3 model names.
- Volatile facts (term origins, tool commands, paper claims) are dated "as of Oct 2026" and attributed. Contested origins are presented as contested.
- Cross-guide links point at the file only (guides have no hash routing). Name the chapter in the link text, e.g. `<a href="gen-ai-hero-to-architect-guide.html">Part 2 · Ch 2 Context as Working Memory</a>`.
- Accent: `--accent: #f472b6` (dark) / `#db2777` (light), pink, distinct from Parts 1–3.
- Content style matches Part 3: `<div class="card">` per subsection, `<h2>N.k Title</h2>`, `<h4 class="ph">` mini-headings, point-wise `<li>`, second person, short sentences.
- **Source of truth:** fragments in `.build/` until Task 9 Step 1 runs the final assemble. After that the assembler is retired, the HTML is the only source, and Task 9/10 edits go straight into the HTML. Never run the assembler after `build-diagrams.mjs`.
- Wherever the retry rule appears, write the literal phrase `max 3 attempts`.
- Work on branch `docs/harness-engineering-guide`, never `main`.
- Commits: `feat(harness-eng): …` for scaffold, `docs(harness-eng): …` for content, `fix(harness-eng): …` for fixes. Each ends with the `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` trailer.

## Review Focus

1. **Sidebar target with no section** (or a section with no nav item): clicking it throws in `go()`. Pinned by the parity check in Task 9.
2. **Diagrams unreadable in light mode** (hard-coded colours): pinned by `lintSvg` in the build plus the light/dark screenshot check in Task 9.
3. **Leak through a fragment or diagram source**: the per-commit gate covers staged files. Task 9 also runs it over the whole tree for files this plan creates.
4. **Six-question drift**: a chapter that paraphrases or reorders the six questions. Pinned by the exact-string count in Task 9.
5. **`netpush` contradictions across Parts written in parallel** (e.g. retry count, deploy order): pinned by the canon grep in Task 9 for `max 3`, `core first` and `plan.yaml`.

---

## File Structure

- Create `gen-ai-harness-engineering-guide.html`: the guide (shell, then assembled).
- Create `.build/harness-engineering/part{0..6}.html` and `ref.html`: per-Part section fragments. Git-ignored via `.git/info/exclude`; never committed.
- Create `scripts/assemble-guide.mjs`: splices fragments into the shell.
- Create `diagrams/harness-engineering/_registry.mjs` and 8 `*.mmd` files.
- Create `docs/superpowers/plans/2026-10-28-harness-sources.md`: the verified source list.
- Modify `README.md` (Gen AI table row), `cspell.json` (new words), and Parts 1–3 HTML (forward link to Part 4).
- Create `REVIEW-gen-ai-harness-engineering-guide-R1.md`: guide review (repo-root convention).

### Section ids (fixed — every task uses exactly these)

| Fragment | Ids |
|---|---|
| part0 | `welcome` (class `section active`), `map` |
| part1 | `ch1-not-the-agent`, `ch2-code-vs-model`, `ch3-anatomy` |
| part2 | `ch4-sees`, `ch5-can-do`, `ch6-state`, `ch7-checker` |
| part3 | `ch8-loop`, `ch9-budgets`, `ch10-background` |
| part4 | `ch11-graph`, `ch12-humans` |
| part5 | `ch13-failure-map`, `ch14-self-improving`, `ch15-hype` |
| part6 | `ch16-netpush`, `ch17-audit` |
| ref | `ref-cheat`, `ref-decisions`, `ref-glossary`, `ref-reading` |

### Chapter template (every chapter except Ch 15)

```
<section id="…" class="section">               ← exact attribute order (parity check relies on it)
  <h1 …>N. Title</h1>                            ← same inline style as Part 3 h1
  <span class="q-tag">What does it remember?</span>   ← 1–2 tags from the six questions
  <span class="hype-tag solid|partial|rebrand|none">…</span>  ← only on chapters that introduce a named term
  <p class="lede">What you'll learn: …</p>
  <div class="card"> <h2>N.1 …</h2> … </div>    ← 3–6 subsections
  <div class="np"><div class="np-label">netpush</div><ul>…</ul></div>   ← running-example tie-in
  <div class="skill-box"><div class="skill-label">What this means for your skill</div><ul>3–5 questions to ask of a skill</ul></div>
  <div class="see-also">…</div>
</section>
```

The first subsection of a chapter that overlaps Part 2 is a one-paragraph recap plus a link (see the briefs).

---

### Task 1: Shell, components, nav, assembler

**Files:** Create `gen-ai-harness-engineering-guide.html` and `scripts/assemble-guide.mjs`. Modify `.git/info/exclude`.

**Produces:** the shell with markers `<!--PART:part0-->…<!--/PART:part0-->` through `part6` and `ref`, in order, inside `.main`. The CSS classes `q-tag`, `hype-tag` (`.solid` green, `.partial` orange, `.rebrand` purple, `.none` red), `np`/`np-label`, `skill-box`/`skill-label`. The sidebar with all 23 targets. `node scripts/assemble-guide.mjs harness-engineering`.

- [ ] Step 0: `git switch -c docs/harness-engineering-guide`.
- [ ] Step 1: `cp gen-ai-architect-to-leader-guide.html gen-ai-harness-engineering-guide.html`. Delete every `<section …>…</section>` inside `.main` and put the 8 empty marker pairs there instead. Remove the `.lens-tag` and `.case` CSS.
- [ ] Step 2: Set `<title>Gen AI for Developers: Harness, Loop &amp; Graph Engineering</title>`, the sidebar brand `Harness, Loop &amp; Graph`, the tagline `Part 4 — designing the system around the model`, and the accent from Global Constraints (both theme blocks).
- [ ] Step 3: Add the four component CSS rules, using `var(--…)` tokens only and a `body.light` variant where Part 3 has one for `.case`.
- [ ] Step 4: Rebuild the sidebar with 8 groups: Front Matter, Part 1 · First Principles, Part 2 · Harness Engineering, Part 3 · Loop Engineering, Part 4 · Graph Engineering, Part 5 · Operating & Evolving, Part 6 · Putting It to Work, Reference. It has one `nav-item` per section id from the table, labelled with the spec §7 chapter titles.
- [ ] Step 5: Write `assemble-guide.mjs <slug>`. For each `<!--PART:x-->…<!--/PART:x-->` in `gen-ai-<slug>-guide.html`, replace the inner text with `.build/<slug>/x.html` (empty if the file is missing). It is idempotent, and it exits non-zero if a marker is unpaired.
- [ ] Step 6: `printf '.build/\n' >> .git/info/exclude && mkdir -p .build/harness-engineering`. Then run `git check-ignore -q .build/harness-engineering/part0.html && echo ignored`. Expected: `ignored`.
- [ ] Step 7: Run the assembler twice. Expected: no diff between the two runs (`shasum` equal). Open the file in Chrome headless (`--screenshot`) and expect the sidebar to render with no sections. (Screenshot convention for all tasks: to capture section `x`, write a temp copy to the scratchpad with `class="section active"` moved from `welcome` to `x`, plus `class="light"` on `body` for light mode. Never call `go()` headlessly, because it relies on the global `event`.)
- [ ] Step 8: Run the leak gate, then commit `feat(harness-eng): scaffold Part 4 shell, components, nav, assembler`.

### Task 2: Verify the source list (spec §11, risks R1–R3)

**Files:** Create `docs/superpowers/plans/2026-10-28-harness-sources.md`.

**Produces:** the only citation list content tasks may use. Each entry has: title · URL · date · one-line verified claim · `solid` or `secondary`.

- [ ] Step 1: Fetch each source in spec §11, including every arXiv abstract page. Keep only those that resolve *and* support the claim the spec attributes to them. Record dropped ones under "Dropped" with the reason.
- [ ] Step 2: Search for and record the primary source for the "three loops" framing. If none is found, write `three-loops: unattributed`.
- [ ] Step 3: Check the current Claude Code docs for `/loop`, scheduled routines, hooks, skills and subagents. Record each as `exists (as of 2026-10-28)` with its doc URL.
- [ ] Step 4: Record the term timeline (harness, loop and graph engineering) with every conflicting origin claim side by side.
- [ ] Step 5: Run the leak gate, then commit `docs(harness-eng): verified source list for Part 4`.

### Tasks 3–8: Content fragments (one task per Part; can run in parallel once Tasks 1–2 are done)

Each task writes **only** its own `.build/harness-engineering/<fragment>.html`. It does not touch the HTML, run the assembler, or commit. Each section follows the chapter template and cites only `harness-sources.md`. Each task places diagram markers as `<figure class="diagram"><!--DIAGRAM:<id>--><!--/DIAGRAM:<id>--><figcaption class="ann">…</figcaption></figure>`, using the ids listed in its brief.

Per-task self-check on the fragment only (replace the ids):
```bash
g=.build/harness-engineering/<fragment>.html
o=$(grep -o "<div" $g|wc -l); c=$(grep -o "</div>" $g|wc -l); echo "div $o/$c"
for id in <ids>; do grep -q "id=\"$id\" class=\"section" $g && echo "ok $id" || echo "MISSING $id"; done
grep -inEf .git/info/leak-terms $g && echo LEAK || echo "no leak"
```
Expected: `div N/N` balanced, `ok` for each id, and `no leak`.

**Integration (after all of Tasks 3–8):** run `node scripts/assemble-guide.mjs harness-engineering`, then run the self-check loop above against the HTML with all 23 ids. Run the leak gate, then commit `docs(harness-eng): write Parts 0–6 and reference`.

#### Task 3: part0 + part1 (Welcome, Map, Ch 1–3)
- **welcome:** who it's for; prerequisites are Parts 1–2, with Part 3 optional; the promise (understand now, upgrade skills later); how to read it; a series cross-link card to Parts 1–3.
- **map:** Agent = Model + Harness; the six-question table; the outer loop; the term timeline from `harness-sources.md`; "meet netpush" with the §6.1 table rendered as `.tbl`. Diagrams: `harness-map`, `term-timeline`.
- **ch1-not-the-agent:** what a raw model can't do alone; harness defined; the prompt → context → harness lineage; why the nested view beats the "eras" view (attributed). Hype tag `solid` for harness engineering.
- **ch2-code-vs-model** *(flagship)*: the determinism dial; a rule table of what must be code (parse, match, order, assert, commit) and what should be the model (synthesis, judgement, summarising); failure costs in both directions. netpush v1 box: **canon gap 1** (deploy matched `grep -i error` and missed an abort string), with a pointer to Ch 16. Diagram: `determinism-dial`.
- **ch3-anatomy:** each of the six questions as a component with 2–3 design levers; how real coding agents implement each, citing only verified study claims. Diagram: `harness-components`.

#### Task 4: part2 (Ch 4–7)
- **ch4-sees:** recap plus link to Part 2 Ch 2; progressive disclosure; skills as on-demand context; what to keep out; context vs. state boundary (→ Ch 6).
- **ch5-can-do:** recap plus link to Part 2 Ch 3; tool surface shape (few sharp tools vs. many); blast radius; the commit-with-auto-rollback pattern in invented, vendor-neutral syntax; read-only workers.
- **ch6-state:** files as memory; `plan.yaml` and reports as shared state; checkpoints; `triage` resuming after compaction; idempotent steps. netpush v1 box: **canon gap 3** (no idempotency marker).
- **ch7-checker:** the loop is only as good as its judge; deterministic checks; dry-run limits ("parse-passing is not commit-passing"); assert on state, not exit codes; LLM-as-judge traps; reward hacking; keeping the checker outside what the agent can edit. Recap plus link to Part 2 Ch 10. netpush v1 box: **canon gap 2** (verify checker editable by the generate agent). Diagram: none.

#### Task 5: part3 (Ch 8–10)
- **ch8-loop:** recap plus link to Part 2 Ch 1; goal → act → observe → adjust; who prompts when you don't; retry policy (netpush generate↔dry-run max 3, then escalate); exit conditions; the three-loops framing, attributed only per `harness-sources.md`. Hype tag for loop engineering. Diagram: `engineered-loop`.
- **ch9-budgets** *(short, 2–3 subsections)*: token, time, iteration and cost budgets as stop rules; fail loud; netpush budget rule.
- **ch10-background** *(short)*: unattended loops (scheduled routines, `/loop`, CI-triggered agents), only as verified in Task 2 Step 3; notifications, kill switches, idempotency (→ Ch 6).

#### Task 6: part4 (Ch 11–12)
- **ch11-graph:** recap plus link to Part 2 Ch 11; nodes (code, LLM call, agent, human); edges and conditions; typed shared state; fan-out/fan-in; orchestrator–worker; stop-on-first-failure with no blind rollback; when a loop should become a graph. Hype tag for graph engineering (practice older than the label). Diagram: `netpush-graph`.
- **ch12-humans** *(short)*: gates as graph nodes; where gates belong (irreversible edges); what the approver must see (a diff, not raw config); rubber-stamp failure; `gen-parser` propose-then-generate. netpush v1 box: **canon gap 4**.

#### Task 7: part5 (Ch 13–15)
- **ch13-failure-map:** recap plus link to Part 2 Ch 1 (bad loops, failure modes) and Ch 9; a table with columns symptom → layer (model or one of the six questions) → first thing to change, ≥ 14 rows with ≥ 2 per layer. Diagram: `failure-flow`.
- **ch14-self-improving:** traces in, harness edits out; `crash-doctor` `lessons.md`; eval-gated harness changes; evaluator and permissions outside the evolving loop; research directions, hedged and from verified sources only. Diagram: `outer-loop`.
- **ch15-hype** *(no netpush or skill box)*: a table with columns term → plain meaning → hype-tag → which of the six questions. Rows: prompt, context, harness, loop, graph, flow, eval, spec, intent, environment engineering, plus any extra term verified in Task 2.

#### Task 8: part6 + ref (Ch 16–17, Reference)
- **ch16-netpush:** the full walkthrough through the six questions, loop, graph, checker, state and gates. It reveals and fixes all four canon gaps, each with before/after, and shows what was already strong.
- **ch17-audit:** the audit rubric as a `.tbl`: six questions × (what to look for · red flags · 0–2 score), plus a model row. A worked audit of `gen-parser` gives findings only, and ends with "how to run this on your own skill" steps.
- **ref-cheat:** one screen: the spine, the six questions, code-vs-model rules, loop stop rules, graph patterns.
- **ref-decisions:** three decision flows as `.stp` blocks: code or model? loop or graph? where does the human go?
- **ref-glossary:** A–Z, ≥ 40 terms, each one sentence.
- **ref-reading:** grouped Solid / Secondary from `harness-sources.md` with dates.

### Task 9: Diagrams, series wiring, whole-guide gates

**Files:** Create `diagrams/harness-engineering/_registry.mjs` and `{harness-map,term-timeline,determinism-dial,harness-components,engineered-loop,netpush-graph,failure-flow,outer-loop}.mmd`. Modify `README.md`, `cspell.json`, and the three Part 1–3 HTML files.

- [ ] Step 1: Write the 8 `.mmd` files and the registry, all `kind: 'mermaid'`, each title one sentence. `term-timeline` may use Mermaid `timeline`. Use Mermaid syntax only; theming comes from `_theme.json`.
- [ ] Step 2: Run `node scripts/assemble-guide.mjs harness-engineering` one last time. This is the **final assemble**: from here on, edit the HTML only. Then run `node scripts/build-diagrams.mjs harness-engineering`. Expected: `injected 8 diagram(s)` and no lint errors.
- [ ] Step 3: Series wiring. Add a README Gen AI row: `Gen AI: Harness, Loop & Graph Engineering` · the file link · a one-line description. Add a Part 4 forward link to each Part 1–3 HTML, following Part 2's existing "Continue to Part 3" sentence pattern and its reference card.
- [ ] Step 4: Gates. Each must pass:
  - Nav parity: the set of `go('x')` targets equals the set of `id="x" class="section` ids. Both have 23 entries.
  - Div balance.
  - Six questions: `grep -c 'What does it remember?'` ≥ 6, and the same for each of the other five strings.
  - Canon: `grep -c 'max 3 attempts'` ≥ 2; `core first` appears; nothing like `max [0-9]` contradicts it (`grep -nE 'max(imum)? [0-9]+ (retries|attempts)'` reviewed by hand).
  - Leak gate over the guide, diagrams, README, plan, spec and sources file: `grep -inEf .git/info/leak-terms <files>` prints nothing.
  - cspell: `npx -y cspell@8.14.4 --no-progress gen-ai-harness-engineering-guide.html`. Add genuine terms to `cspell.json` `words` and fix real typos.
  - Render: headless Chrome screenshots of `welcome`, `ch11-graph` and `ch16-netpush` in dark and light (toggle by adding `class="light"` to `body` in a temp copy). Read the screenshots and confirm the diagrams are legible. Use a 390px-wide screenshot to check there's no horizontal scroll.
- [ ] Step 5: Commit `docs(harness-eng): diagrams, series wiring, gates`.

### Task 10: Super-review the guide and fix

- [ ] Step 1: Run `super-review` on `gen-ai-harness-engineering-guide.html`. The file is over 800 lines, so it uses split agents, given the six questions and §6.1 canon as reference values. Save to repo root as `REVIEW-gen-ai-harness-engineering-guide-R1.md` (repo convention).
- [ ] Step 2: Apply the fixes in order HIGH → MED → LOW. Then re-run the Task 9 Step 4 gates.
- [ ] Step 3: Run the leak gate, then commit `fix(harness-eng): apply super-review R1`.
