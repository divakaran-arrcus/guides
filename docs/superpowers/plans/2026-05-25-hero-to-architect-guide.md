# Hero to Architect Guide Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. The `guide-builder` skill should also be loaded — it has the canonical component vocabulary for this repo's HTML guides.

**Goal:** Build a single-page HTML companion to `gen-ai-zero-to-hero-guide.html` titled "Gen AI for Developers: Hero to Architect" — 17 deep chapters across foundations / methodology / mastery / worked-example / reference.

**Architecture:** One self-contained `hero-to-architect-guide.html` file. Sidebar-driven SPA pattern (sections with `display: none`, JS `go(id)` activates one). Reuses v1's chrome (fonts, color tokens, info boxes, comparison grids). Each chapter = one HTML `<section>` element with id matching the sidebar nav-item.

**Tech Stack:** Vanilla HTML + CSS + JS. No build step. Google Fonts (DM Sans, Source Serif 4, JetBrains Mono).

**Reference inputs:**
- Spec: `docs/superpowers/specs/2026-05-25-hero-to-architect-guide-design.md`
- Pattern source: `gen-ai-zero-to-hero-guide.html` (copy chrome, adapt content)

**Worked example throughout chapters 6–11 and 17:** `gitlog-report` CLI (reads `git log`, produces per-author activity summary).

---

## File Structure

Single artifact:
- Create: `/Users/divakaran/arrcus_workspace/guides/hero-to-architect-guide.html`
- Modify: `/Users/divakaran/arrcus_workspace/guides/README.md` (add row to guides table — final task)

No other files. Embedded CSS + JS in the HTML file (matches the rest of the repo's guides).

---

## Conventions used in this plan

**Each "chapter task" follows the same pattern:**
1. Add chapter `<section id="..">` scaffold (h1, "what you'll learn" preamble, h2 placeholders for each subsection)
2. Fill in subsection content in groups of 3–5 (one step per group)
3. Add "See also →" cross-link grid at end of chapter
4. Render in browser, navigate to chapter, verify layout, dark/light toggle, no console errors
5. Commit with message `docs(hero-to-architect): write Ch N — <title>`

**Content brief per subsection (used inside steps):** `<subsection-number> <title>` — 2–4 paragraphs covering: [concepts], [examples/snippets if applicable], [callout pattern if applicable: "Kilo primary, Claude/Gemini alt" / "Gotcha" / "Mini-walkthrough: gitlog-report"]. The brief lists what content to include; the writer produces the prose.

**Commit message convention:** structural / scaffolding tasks (Task 1 scaffold, Task 2 sidebar) use `feat(hero-to-architect): …`. Content tasks (Task 3 onward — front matter, chapters, reference pages) use `docs(hero-to-architect): …`. Fix-up commits use `fix(hero-to-architect): …`.

**Tool callout component:** wherever a chapter shows a tool-specific example, wrap the primary tool in main content and place the alt-tool equivalent inside a `<aside class="tool-alt">` box. CSS class to be added in Task 1.

**Mini-walkthrough component (Ch 6–11 only):** at the end of the chapter (before "See also"), add a `<div class="walkthrough">` containing 3–5 short bullets showing how this chapter's methodology was applied to `gitlog-report`. Cumulative — each chapter's walkthrough builds on the previous.

---

## Task 1: Scaffold the HTML file from v1's chrome

**Files:**
- Create: `/Users/divakaran/arrcus_workspace/guides/hero-to-architect-guide.html`
- Read: `/Users/divakaran/arrcus_workspace/guides/gen-ai-zero-to-hero-guide.html` (source pattern)

- [ ] **Step 1: Copy v1 file as starting point**

```bash
cp /Users/divakaran/arrcus_workspace/guides/gen-ai-zero-to-hero-guide.html \
   /Users/divakaran/arrcus_workspace/guides/hero-to-architect-guide.html
```

- [ ] **Step 2: Update `<title>` and sidebar brand**

Edit `hero-to-architect-guide.html`:
- `<title>Gen AI for Developers: Hero to Architect</title>`
- Sidebar `<h1>` → `Hero to Architect`
- Sidebar tagline `<p>` → `Companion guide — agentic coding mastery`

- [ ] **Step 3: Introduce a `--accent` token for the guide-specific accent**

v1 has **two** `:root` blocks: a default (dark-mode) one and a `body.light` override block. Add `--accent` to **both**.

In the default `:root` block (find it with `grep -n '^  :root' gen-ai-zero-to-hero-guide.html | head -1`), add:

```css
--accent: #c084fc;  /* purple — distinguishes v2 from v1 at a glance */
```

In the `body.light` override block (find it with `grep -n 'body.light {' gen-ai-zero-to-hero-guide.html` — the override that flips palette tokens for light mode), add a slightly deeper purple for better contrast on light backgrounds:

```css
--accent: #9333ea;
```

Then update two specific rules to use the new token (do not touch any other `var(--cyan)` or `var(--blue)` usages):

1. `.sidebar-brand h1` — change `color: var(--cyan);` → `color: var(--accent);` (in both dark and light rules if v1 has both).
2. `.nav-item.active` — change `border-left: 2px solid var(--blue);` → `border-left: 2px solid var(--accent);` in both rules. Locate them with `grep -nE '\.nav-item\.active' gen-ai-zero-to-hero-guide.html` (expect two hits — dark + light) rather than relying on hardcoded line numbers.

Keep `--cyan` and `--blue` at their v1 values — only `.sidebar-brand h1` and `.nav-item.active` border use the new `--accent`. This way the family resemblance holds and the rest of the guide (code text, `.sec-num`, tables, info-boxes, walkthrough h3, see-also links) retains the v1 cyan tone.

- [ ] **Step 4: Strip v1's sidebar nav-groups and v1's content sections**

Delete all `<div class="nav-group">...</div>` blocks inside `<aside class="sidebar">` and all `<section class="section">...</section>` blocks inside `<main class="content">`. Leave the chrome intact: sidebar shell + brand block, content shell, theme toggle button, and the closing `<script>` block at the end of `<body>`.

- [ ] **Step 5: Add new component CSS — `tool-alt` callouts, `walkthrough` boxes, `card-grid`**

Append to the existing `<style>` block (just before `</style>`):

```css
/* Tool-alt callout (e.g., "Claude Code equivalent") */
aside.tool-alt {
  background: var(--surface2);
  border-left: 3px solid var(--purple);
  border-radius: 8px;
  padding: 14px 18px;
  margin: 14px 0;
  font-size: 14px;
}
aside.tool-alt::before {
  content: "Alt tool →";
  display: inline-block;
  font-family: 'JetBrains Mono', monospace;
  font-size: 10px;
  font-weight: 700;
  color: var(--purple);
  letter-spacing: 1.5px;
  margin-right: 8px;
}

/* gitlog-report mini-walkthrough box — cyan-to-accent (purple) gradient
   matches the welcome diagram's cyan/accent identity */
div.walkthrough {
  background: linear-gradient(135deg,
    color-mix(in srgb, var(--cyan) 6%, transparent),
    color-mix(in srgb, var(--accent) 6%, transparent));
  border: 1px solid var(--border-light);
  border-radius: 10px;
  padding: 18px 22px;
  margin: 24px 0;
}
div.walkthrough h3 {
  font-family: 'JetBrains Mono', monospace;
  font-size: 12px;
  text-transform: uppercase;
  letter-spacing: 1.5px;
  color: var(--cyan);
  margin-bottom: 10px;
}

/* Card grid for adjacent-tool tours */
div.card-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
  gap: 14px;
  margin: 18px 0;
}
div.card-grid > div.tool-card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 16px;
}
div.card-grid > div.tool-card h4 {
  color: var(--blue);
  margin-bottom: 8px;
}
div.card-grid > div.tool-card .tool-meta {
  font-size: 12px;
  color: var(--text-muted);
  margin-top: 8px;
}

/* See-also link grid */
div.see-also {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 10px;
  margin-top: 32px;
  padding-top: 18px;
  border-top: 1px solid var(--border);
}
div.see-also a {
  color: var(--cyan);
  font-size: 13px;
  text-decoration: none;
}
div.see-also a:hover { text-decoration: underline; }
```

- [ ] **Step 6: Add empty placeholder `<section>` for welcome so the page renders without errors**

First, confirm v1's active-section class name (the snippet below assumes `section active`):

```bash
grep -oE 'class="[^"]*active[^"]*"' /Users/divakaran/arrcus_workspace/guides/gen-ai-zero-to-hero-guide.html | sort -u
```

Expected: `class="section active"`. If v1 uses a different convention (e.g., `is-active`, `current`), use that instead.

Then insert inside `<main class="content">` (preserve the `active` class so the section is visible on first load):

```html
<section id="welcome" class="section active">
  <h1>Gen AI for Developers: Hero to Architect</h1>
  <p>Scaffold pending content.</p>
</section>
```

- [ ] **Step 7: Verify in browser**

Open `hero-to-architect-guide.html` in a browser. Verify:
- Page renders (no JS console errors)
- Dark/light mode toggle works
- New accent color visible in sidebar brand
- Scaffold welcome section visible

- [ ] **Step 8: Commit**

```bash
cd /Users/divakaran/arrcus_workspace/guides
git add hero-to-architect-guide.html
git commit -m "feat(hero-to-architect): scaffold guide chrome with adjusted accent"
```

---

## Task 2: Build complete sidebar navigation

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Replace empty sidebar with all nav groups**

Insert inside `<aside class="sidebar">` after `<div class="sidebar-brand">...</div>`:

```html
<div class="nav-group">
  <div class="nav-group-label">Front Matter</div>
  <button class="nav-item active" onclick="go('welcome')">Welcome</button>
  <button class="nav-item" onclick="go('map')">The Map</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 1 — Foundations</div>
  <button class="nav-item" onclick="go('ch1-loop')">1. The Agentic Coding Loop</button>
  <button class="nav-item" onclick="go('ch2-context')">2. Context as Working Memory</button>
  <button class="nav-item" onclick="go('ch3-tools')">3. Tools, Side Effects &amp; the Agent's World</button>
  <button class="nav-item" onclick="go('ch4-models')">4. Model Selection &amp; Cost Engineering</button>
  <button class="nav-item" onclick="go('ch5-surface')">5. IDE vs Terminal</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 2 — Methodology</div>
  <button class="nav-item" onclick="go('ch6-spec')">6. Spec-Driven Dev (OpenSpec &amp; SpecKit)</button>
  <button class="nav-item" onclick="go('ch7-plan')">7. Plan-Driven Execution</button>
  <button class="nav-item" onclick="go('ch8-tdd')">8. TDD with Agents</button>
  <button class="nav-item" onclick="go('ch9-debug')">9. Debugging with Agents</button>
  <button class="nav-item" onclick="go('ch10-review')">10. Safety, Review &amp; Verification</button>
  <button class="nav-item" onclick="go('ch11-parallel')">11. Multi-Agent &amp; Parallel</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 3 — Mastery</div>
  <button class="nav-item" onclick="go('ch12-kilo')">12. Kilo Code Deep Dive</button>
  <button class="nav-item" onclick="go('ch13-claude')">13. Claude Code Deep Dive</button>
  <button class="nav-item" onclick="go('ch14-gemini')">14. Gemini CLI Deep Dive</button>
  <button class="nav-item" onclick="go('ch15-superpowers')">15. Superpowers &amp; Skills Authoring</button>
  <button class="nav-item" onclick="go('ch16-mcp')">16. MCP Servers</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 4 — Putting It Together</div>
  <button class="nav-item" onclick="go('ch17-worked')">17. Worked Example: gitlog-report</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Reference</div>
  <button class="nav-item" onclick="go('ref-decisions')">Decision Frameworks</button>
  <button class="nav-item" onclick="go('ref-cheats')">Cheat Sheets</button>
  <button class="nav-item" onclick="go('ref-glossary')">Glossary A–Z</button>
  <button class="nav-item" onclick="go('ref-reading')">Further Reading</button>
</div>
```

- [ ] **Step 2: Verify sidebar renders correctly**

Open the file. Verify all 23 nav buttons appear under their group labels. Only the `welcome` section exists at this point; clicking any other nav-item will blank the content area (expected — *not* a regression). The active highlight should still move on click.

- [ ] **Step 3: Commit**

```bash
git add hero-to-architect-guide.html
git commit -m "feat(hero-to-architect): build complete sidebar navigation"
```

---

## Task 3: Front matter — Welcome + The Map

**Files:**
- Modify: `hero-to-architect-guide.html`
- Read: `docs/superpowers/specs/2026-05-25-hero-to-architect-guide-design.md` §4 (v1-vs-v2 comparison rows) and §5 (framing rules)

- [ ] **Step 0: Preflight — open the spec to §4 and §5 before writing**

Have the spec open in the other pane. The welcome section copies the comparison rows from §4 and the framing rules from §5 verbatim (or near-verbatim); do not paraphrase from memory.

- [ ] **Step 1: Replace scaffold `welcome` section with full content**

Replace the placeholder `<section id="welcome">` with a complete welcome section. **Preserve `class="section active"` on the new section** so it remains visible on first load. Cover:
- **Who this is for** — developers picking up Claude Code, Kilo Code, or Gemini CLI; assumes v1 read
- **What's different from v1** — depth over breadth; methodology over taxonomy; reference + training, not just training
- **How to read it** — linear training path vs. dip-in reference; the three layers
- **Prerequisites** — v1 mental model, basic git/CLI fluency, comfort installing tools

Include a comparison table (v1 vs v2) with the rows from spec §4.

- [ ] **Step 2: Add `map` section**

Insert after `welcome`:

```html
<section id="map" class="section">
  <h1>The Map</h1>
  <!-- Content -->
</section>
```

Content covers:
- The three layers visually (Foundations → Methodology → Mastery) — use a styled vertical diagram with a `var(--cyan)` to `var(--accent)` gradient (cyan-to-purple)
- Where the worked example threads through (Ch 6–11 + Ch 17)
- How to use as reference (Reference section entry points)
- Framing rules from spec §5 in a callout box

- [ ] **Step 3: Verify rendering**

Open file. Click Welcome and The Map nav items. Verify content displays, no console errors, both light and dark themes look right.

- [ ] **Step 4: Commit**

```bash
git add hero-to-architect-guide.html
git commit -m "docs(hero-to-architect): write Welcome and The Map"
```

---

## Task 4: Chapter 1 — The Agentic Coding Loop

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter section scaffold**

Insert after the `map` section:

```html
<section id="ch1-loop" class="section">
  <h1>1. The Agentic Coding Loop</h1>
  <p class="lede">What you'll learn: how an agent actually works under the hood — the plan/act/observe cycle, what differs from chat, where it breaks, and how to read the loop in real time.</p>

  <h2>1.1 From chat to agent — the fundamental shift</h2>
  <h2>1.2 The loop: plan → act → observe → repeat</h2>
  <h2>1.3 Anatomy of one agent turn</h2>
  <h2>1.4 What the agent "sees" vs what you see</h2>
  <h2>1.5 Determinism, non-determinism, and the temperature knob</h2>
  <h2>1.6 The control problem</h2>
  <h2>1.7 Plan mode vs auto mode vs interactive</h2>
  <h2>1.8 Anatomy of a bad loop</h2>
  <h2>1.9 Mental model: agents are interns with very fast hands</h2>
  <h2>1.10 Common failure modes and early-warning signs</h2>
  <h2>1.11 Hands-on: trace one real agent turn</h2>
</section>
```

- [ ] **Step 2: Fill subsections 1.1–1.4 (the "what is happening" cluster)**

Content briefs:
- **1.1** — Reframe chat (one Q, one A) vs agent (Q, then a loop of tool calls until done). 2–3 paragraphs. Reference v1's "What is an AI Agent" for breadth, this for depth.
- **1.2** — The four-phase loop. Diagram: plan → act → observe → repeat with arrows. Each phase explained in one paragraph.
- **1.3** — Walk through a single turn: prompt → (optional) thinking → tool calls → tool results → next turn. Code-style snippet showing JSONL trace of a Read+Edit+Bash sequence.
- **1.4** — The agent's view: system prompt + conversation + tool results. Your view: terminal/IDE output. Mismatch is where confusion lives. Concrete example: agent sees `<system-reminder>`, you don't.

- [ ] **Step 3: Fill subsections 1.5–1.7 (the "controlling it" cluster)**

Content briefs:
- **1.5** — Temperature dial, when to bump for creativity (rare in coding). **Note that Claude Code and Gemini CLI don't expose temperature to end-users at the CLI surface — it's fixed/invisible there. The dial matters when calling the underlying APIs directly.** Also non-determinism beyond temperature: tool-call ordering, race conditions in parallel tools.
- **1.6** — Stop conditions: explicit Stop tool, max turns, user interrupt. Human-in-loop patterns. Plan-then-confirm gates.
- **1.7** — A pedagogical three-mode taxonomy (our framing, not a shared standard): Plan (think first, no actions), Auto (just go), Interactive (ask before risky tools). Map back to each tool's actual naming: Claude Code uses Default + Plan mode + a shift-tab "auto-accept edits" toggle; Kilo Code uses Architect / Code / Ask / Debug; Gemini CLI uses slash commands and the `--yolo` flag. **Explicitly tell the reader that the three-mode framing is ours — the tools don't share these labels.**

- [ ] **Step 4: Fill subsections 1.8–1.11 (the "what breaks" + hands-on)**

Content briefs:
- **1.8** — Ratholes (agent goes deep on a tangent), drift (lost the original goal), premature claim-of-done. Show signs of each.
- **1.9** — The mental model that survives: interns with hands. Fast, eager, sometimes wrong, need clear specs.
- **1.10** — Failure mode catalog (8–10 entries) with early-warning signs in a 2-col table.
- **1.11** — Hands-on: prompt Kilo Code and Claude Code with the same simple task. Show the JSONL turn from each. Reader compares.

Add tool-alt callout at 1.7 showing the Kilo/Claude/Gemini equivalents side-by-side.

- [ ] **Step 5: Add See-also grid**

Below 1.11, add:

```html
<div class="see-also">
  <a onclick="go('ch2-context')">→ Ch 2. Context as Working Memory</a>
  <a onclick="go('ch3-tools')">→ Ch 3. Tools, Side Effects &amp; the Agent's World</a>
  <a onclick="go('ch10-review')">→ Ch 10. Safety, Review &amp; Verification</a>
</div>
```

- [ ] **Step 6: Render and verify**

Open in browser. Navigate to Ch 1. Verify all 11 subsections render, code blocks format correctly, see-also links work.

- [ ] **Step 7: Commit**

```bash
git add hero-to-architect-guide.html
git commit -m "docs(hero-to-architect): write Ch 1 — The Agentic Coding Loop"
```

---

## Task 5: Chapter 2 — Context as Working Memory

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 2.1–2.11)**

Same pattern as Task 4 Step 1. Subsection titles from spec §8 Ch 2.

- [ ] **Step 2: Fill 2.1–2.4 (the token economy)**

Content briefs:
- **2.1** — Context window as a working RAM. Numbers: 200K default Claude (Sonnet/Opus), Claude Opus 4.7 also offers a 1M context tier; 1M Gemini; 200K GPT-5 (verify all numbers against current docs). What "in context" actually means.
- **2.2** — Surprising token costs: system prompts, tool schemas, prior tool results, cached system prompts on cache miss. Show a screenshot/diagram of token breakdown.
- **2.3** — Input/output/cached token math. Cache *reads* are ~0.1× the base input cost (often quoted as "10x cheaper"), but cache *writes* cost ~1.25× the base input cost on Anthropic — so amortization matters. Show two formulas: the simplified steady-state hit cost (`input_uncached*X + input_cached*X/10 + output*Y`) and the full first-write cost that includes the write premium. Note that the multipliers vary by provider; cite current Anthropic/OpenAI/Google docs.
- **2.4** — Caching mechanics: prefix-based, breaks on edits to system or earlier messages. Patterns that maximize cache hits.

- [ ] **Step 3: Fill 2.5–2.8 (managing the budget)**

Content briefs:
- **2.5** — Compaction mechanisms differ per tool: Claude Code has automatic compaction near the context limit plus a manual `/compact` slash command; Gemini CLI uses `/compress`; Kilo Code's condensing behavior is built into its session manager (mechanism and command name evolve — **verify the exact compaction surface for each tool before writing**, don't assume symmetry). Common across all: older turns get summarized, recent turns and pinned content stay.
- **2.6** — Long sessions: keep them alive by pinning key files, clearing irrelevant tool output, using sub-agents to offload investigation.
- **2.7** — `/clear` vs continue. Decision: new topic = clear; same task with stale context = clear; mid-task = continue. Mini decision tree. **Note: `/clear` exists in Claude Code and Gemini CLI; Kilo Code's equivalent is the "New Task" button in the side panel, not a slash command.**
- **2.8** — Reading the budget: `/cost` (per-turn spend) and `/context` (budget view) in Claude Code; `ccusage` (third-party npm monitor) for richer historical reporting; Gemini CLI shows token counts in its footer; Kilo Code shows them in the chat panel. **Verify exact commands against current docs before writing — these surfaces shift.**

- [ ] **Step 4: Fill 2.9–2.11 (memory + anti-patterns + hands-on)**

Content briefs:
- **2.9** — Memory systems live *outside* context until referenced. CLAUDE.md (auto-loaded per project), GEMINI.md (same idea), Kilo Code's rules file (likely `.kilocode/rules/` directory — **verify exact path against current Kilo docs before writing**). Show a 3-col side-by-side.
- **2.10** — Anti-patterns: pasting whole files when grep would do; paste-and-pray; log dumps. Concrete examples.
- **2.11** — Hands-on: open a fresh session, run a real task, watch token usage. Snapshot before/after `/compact`.

Add a tool-alt callout at 2.9 showing the three memory file conventions.

- [ ] **Step 5: Add see-also (→ Ch 4 caching, → Ch 13 CLAUDE.md, → Ch 14 GEMINI.md, → Ch 12 Kilo rules)**

- [ ] **Step 6: Render and verify**

- [ ] **Step 7: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 2 — Context as Working Memory"
```

---

## Task 6: Chapter 3 — Tools, Side Effects & the Agent's World

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 3.1–3.11)**

- [ ] **Step 2: Fill 3.1–3.4 (what tools are)**

Briefs:
- **3.1** — A tool is a JSON-schema'd function the model can call. Show schema example.
- **3.2** — The built-in dailies: Read, Edit, Write, Bash, Grep, Glob. One paragraph each on when each is right.
- **3.3** — How the model picks: tool description, schema fit, prior turn context. Why descriptions matter for your own tools (foreshadows Ch 16).
- **3.4** — Reversible vs hard-to-reverse. Tier the built-ins by blast radius (table). Cross-link to verification chapter.

- [ ] **Step 3: Fill 3.5–3.8 (controlling tools)**

Briefs:
- **3.5** — Idempotency: rerunning the same Bash isn't always safe. Show migration/install examples. Retries.
- **3.6** — Permission tiers: ask-per-call, allowlist, allow-edits, plan-only. Compare Claude Code defaults vs Kilo Code's per-category auto-approve checkboxes vs Gemini CLI's `--yolo` / `-y` *flag* (a one-shot launch flag, not a UI mode toggle — call out this asymmetry explicitly).
- **3.7** — Per-project allowlists. Show `.claude/settings.json` with permissions block, and Kilo's equivalent permission rules. Tool-alt callout.
- **3.8** — Auto-mode pros (speed) vs cons (blast). Rules of thumb for when each tool family belongs auto.

- [ ] **Step 4: Fill 3.9–3.11 (sandbox + observability + hands-on)**

Briefs:
- **3.9** — Sandbox spectrum: bare metal → devcontainer → VM → ephemeral container. Cost/safety tradeoffs.
- **3.10** — Observability: tool-call logs, git diff snapshots, hooks, `--output-format=stream-json` (Claude Code specific — Gemini CLI and Kilo Code use different observability surfaces; note the divergence). Where to look when something unexpected lands.
- **3.11** — Hands-on: design a `.claude/settings.json` allowlist for a real project (your repo). Apply same logic to a Kilo rule.

- [ ] **Step 5: See-also (→ Ch 11 worktrees, → Ch 16 MCP)**

- [ ] **Step 6: Render and verify**

- [ ] **Step 7: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 3 — Tools, Side Effects"
```

---

## Task 7: Chapter 4 — Model Selection & Cost Engineering *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

Note: this chapter has 14 subsections (vs 11 for Ch 1–3). Expect more time.

- [ ] **Step 1: Add chapter scaffold (subsections 4.1–4.14)**

- [ ] **Step 2: Fill 4.1–4.6 (the landscape)**

Briefs:
- **4.1** — May 2026 landscape: Claude 4.7 Opus/Sonnet/Haiku, GPT-5 + 5-mini, Gemini 3 Pro/Flash, open-weight (Llama, Qwen, DeepSeek). 1-line capability/cost snapshot per family.
- **4.2** — Three dials: capability, cost, latency. The "fast and cheap or smart and slow" pick.
- **4.3** — Claude family per-use guide. Opus = architect/deep code; Sonnet = daily coder; Haiku = volume tasks. Pricing per million tokens — note that Opus 4.7's 1M context tier has different pricing from its 200K tier. **Verify current pricing against `docs.claude.com` at write-time.**
- **4.4** — GPT family per-use guide. **Verify current pricing and model lineup against `platform.openai.com` at write-time.**
- **4.5** — Gemini family per-use guide. Highlight free-tier mechanics. **Verify current free-tier limits and paid pricing against `ai.google.dev` at write-time.**
- **4.6** — Local: Ollama / LM Studio. Latency win, capability cost. When it makes sense (privacy, cost, offline).

- [ ] **Step 3: Fill 4.7–4.10 (advanced cost levers)**

Briefs:
- **4.7** — Kilo Code BYO routing: per-mode model picks (*example, not Kilo's shipped default*: Architect=Opus, Code=Sonnet, Ask=Haiku/Gemini Flash). Show the custom-modes config snippet — **verify exact filename and schema against current Kilo docs** (the canonical path has historically been `.kilocode/` workspace or `.roomodes`; pin the current name before writing).
- **4.8** — Thinking mode: extended thinking budget, when it pays for itself, when it's waste. Examples.
- **4.9** — Prompt caching as a lever: keep system prompts stable, append rather than insert, monitor cache hit rate.
- **4.10** — Batch API for async (Claude Message Batches, OpenAI Batch, similar from Google). Historical pattern has been ~50% discount on input/output, ~24h SLA — **verify current discount and SLA against each provider's batch docs before writing**, terms have shifted. When to use vs. real-time.

- [ ] **Step 4: Fill 4.11–4.14 (mental model + hands-on)**

Briefs:
- **4.11** — Effective context: nominal 200K/1M ≠ usable. Rule of thumb (model-dependent, evolving): quality often degrades past 50–70% fill — the older "lost-in-the-middle" effect. Gemini 3 Pro's 1M is notably stronger at long-range recall than 2024-era frontier; cite a current needle-in-haystack benchmark rather than asserting a flat percentage.
- **4.12** — Cost-per-feature mental model. Per feature: input × turns × tokens. Example budget: a CRUD endpoint ≈ $X.
- **4.13** — Decision tree: task type → model. Flowchart.
- **4.14** — Hands-on: take your last week's work, estimate token spend, redo with model routing — show savings.

- [ ] **Step 5: See-also (→ Ch 12 Kilo modes, → Ch 13 fast mode, → Ch 14 Gemini free tier)**

- [ ] **Step 6: Render and verify**

- [ ] **Step 7: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 4 — Model Selection & Cost"
```

---

## Task 8: Chapter 5 — IDE vs Terminal: Choosing Your Surface *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

13 subsections including the expanded comparison cards.

- [ ] **Step 1: Add chapter scaffold (subsections 5.1–5.13)**

- [ ] **Step 2: Fill 5.1–5.5 (the three primaries)**

Briefs:
- **5.1** — Two idioms. IDE-resident (lives in your editor, visual). Terminal-resident (lives in your shell, scriptable). Tradeoffs in one paragraph each.
- **5.2** — The three primaries today: Kilo Code (IDE), Claude Code (terminal), Gemini CLI (terminal). One-line positioning each.
- **5.3** — Kilo Code: what it is + first run. Install from VS Code marketplace, sign-in/BYO key, first prompt. Screenshot or ASCII representation of the modes panel.
- **5.4** — Claude Code: what it is + first run. `npm install -g @anthropic-ai/claude-code` then `claude login`, run first prompt in `~/some-project`. **Verify the install command against `docs.claude.com/claude-code` before writing** — Claude Code has shifted between npm and native binary distribution methods historically.
- **5.5** — Gemini CLI: what it is + first run. Install via `npm install -g @google/gemini-cli` (or `npx @google/gemini-cli` for one-off) — **verify package name against `https://github.com/google-gemini/gemini-cli` before writing**, this has shifted historically. Show authentication choices: personal Google account (free tier — currently 1M context, ~1000 req/day; verify current numbers) vs API key (Gemini API) vs Vertex AI. Run first prompt in `~/some-project`.

- [ ] **Step 3: Fill 5.6–5.8 (IDE vs terminal, multiple tools)**

Briefs:
- **5.6** — IDE strengths: visual diffs, inline edits, tab context. Terminal strengths: scriptable, headless, CI, multi-session.
- **5.7** — Claude Code vs Gemini CLI: feature parity matrix, model defaults, plugin/extension ecosystems. **Call out the billing asymmetry up front:** Claude Code is subscription (Pro/Max plan) or API-key billed — there is no Gemini-CLI-style free tier with a personal Google account. Gemini CLI's free tier (1M context, ~1000 req/day with a personal Google account — verify current limits) is genuinely "free for personal use."
- **5.8** — Working in multiple at once — common pairing (Kilo Code for editing + Claude Code for running review/security checks). When this helps.

- [ ] **Step 4: Fill 5.9 (IDE alternatives card grid)**

Use `<div class="card-grid">` with one `<div class="tool-card">` per tool. Six cards: Cursor, Windsurf, Antigravity, Cline/Roo Code, Continue.dev, JetBrains AI Assistant/Junie. Each card has h4 (tool name), "What it is" line, "Who it's for" line, "Key strength" line, "Why not deep here" line, and a `<div class="tool-meta">` with link-to-docs hint.

- [ ] **Step 5: Fill 5.10 (Terminal alternatives card grid)**

Same card-grid pattern. Three cards: Aider, GitHub Copilot CLI/Spark/Workspace, Codex CLI.

- [ ] **Step 6: Fill 5.11–5.13 (decision matrix + hands-on + where next)**

Briefs:
- **5.11** — Decision matrix table: rows are task types (quick edit, multi-file refactor, security review, exploratory build, CI script, doc writing), columns are tools. Cells indicate best/good/avoid.
- **5.12** — Hands-on: pick a small task you'd do today. Run it in two of the three primary tools. Note what felt different.
- **5.13** — **Where next: pointers into Part 3 deep-dives.** One short paragraph of framing prose: "now that you've picked your surface(s), the deep-dives in Part 3 unpack each tool's full feature surface." Then a see-also grid (→ Ch 12 Kilo deep dive, → Ch 13 Claude deep dive, → Ch 14 Gemini deep dive). This counts as the 13th subsection — distinct from the chapter-level "See also" grid added in Step 7.

- [ ] **Step 7: Render and verify**

Pay extra attention: card-grid layout must wrap nicely on narrow screens.

- [ ] **Step 8: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 5 — IDE vs Terminal"
```

---

## Task 9: Chapter 6 — Spec-Driven Development: OpenSpec & SpecKit *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

First methodology chapter — introduces the gitlog-report mini-walkthrough pattern.

- [ ] **Step 1: Add chapter scaffold (subsections 6.1–6.11)**

- [ ] **Step 2: Fill 6.1–6.4 (concept + OpenSpec)**

Briefs:
- **6.1** — Why specs matter when agents code fast: drift is cheap, drift recovery is expensive.
- **6.2** — The contract: spec describes behavior, tests enforce it. Agent reads spec → writes implementation → tests verify. Show triangle diagram.
- **6.3** — OpenSpec model: an `openspec/` workspace at repo root containing `openspec/specs/` (canonical), `openspec/changes/<id>/proposal.md` + `openspec/changes/<id>/specs/` (delta), and `openspec/changes/archive/` after accept. **Verify exact layout against current OpenSpec docs before writing** — repo structure has shifted historically (similar freshness check pattern as Ch 14).
- **6.4** — OpenSpec walk-through using `gitlog-report` as the example (consistent with the running example introduced fully in 6.11). Change *creation* is done by the AI assistant via a slash command (e.g. `/opsx:propose add-gitlog-report` — **verify the current slash command in the OpenSpec README**), which writes the proposal file under `openspec/changes/add-gitlog-report/`. CLI surface is for lifecycle only: `openspec list`, `openspec show <id>`, `openspec validate`, `openspec archive <id>`. Show the proposal file, the delta spec under `openspec/changes/add-gitlog-report/specs/`, and how the agent reads both during implementation. **Verify all command names and file paths against current docs at `https://github.com/Fission-AI/OpenSpec` before writing.**

- [ ] **Step 3: Fill 6.5–6.8 (SpecKit + comparison)**

Briefs:
- **6.5** — SpecKit model: slash commands `/specify`, `/plan`, `/tasks`, `/implement`. Each command writes to a feature-scoped directory — list the actual artifacts (likely `spec.md`, `plan.md`, `tasks.md` inside `.specify/specs/<feature>/` or `specs/<NNN-feature>/` depending on version). **Verify the exact directory pattern and file names against current SpecKit docs before writing** — these have shifted historically.
- **6.6** — SpecKit walk-through. Same example as 6.4 in SpecKit terms. Show file output of each slash command.
- **6.7** — Side-by-side comparison table (8–10 rows): philosophy, file structure, change tracking, slash commands, archive model, multi-tool support, etc.
- **6.8** — Decision matrix: which fits when. OpenSpec = manual + git-native + tool-agnostic; SpecKit = guided + slash-command-driven + Claude/Gemini-first.

- [ ] **Step 4: Fill 6.9–6.10 (lifecycle + antipatterns)**

Briefs:
- **6.9** — Living with specs: when the implementation diverges, when to refactor the spec vs the code. Archive discipline.
- **6.10** — Anti-patterns: speccing trivial CRUD; spec ≫ implementation effort; specs that drift untouched.

- [ ] **Step 5: Fill 6.11 — Mini-walkthrough: gitlog-report spec**

Add a `<div class="walkthrough">` showing:
- The brief: `gitlog-report` — CLI that reads git log and produces per-author summary
- 5–7 bullet acceptance criteria
- A snippet of the OpenSpec proposal markdown for it
- A snippet of the SpecKit `/specify` output for the same tool
- "Next chapter: we plan it"

- [ ] **Step 6: See-also (→ Ch 7 plan, → Ch 8 tests, → Ch 17 worked example)**

- [ ] **Step 7: Render and verify**

- [ ] **Step 8: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 6 — Spec-Driven Development"
```

---

## Task 10: Chapter 7 — Plan-Driven Execution

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 7.1–7.11)**

- [ ] **Step 2: Fill 7.1–7.4 (concepts)**

Briefs:
- **7.1** — Plan ≠ spec. Spec = what the system does (durable). Plan = how we'll build it (transient). Different audiences, different lifecycles.
- **7.2** — The four-step rhythm: brainstorm (explore) → spec (commit to what) → plan (commit to how) → execute. Diagram.
- **7.3** — Good plan anatomy: numbered tasks, files-touched per task, code blocks for each step, commit cadence. Reference superpowers:writing-plans skill.
- **7.4** — Plan mode in Claude Code: how to enter (shift-tab cycling or `/plan`), the approval gate (`ExitPlanMode` tool required to proceed), and what's restricted: `Edit`, `Write`, `Bash` and other side-effecting tools blocked; `Read`, `Grep`, `Glob`, `WebFetch` allowed. Compare to Kilo Architect mode — **important asymmetry: Kilo Architect plans *and writes files* (it's a write-capable orchestrator); Claude plan mode forbids any writes.** Spell this out so the reader doesn't equate them.

- [ ] **Step 3: Fill 7.5–7.8 (workflows)**

Briefs:
- **7.5** — Plan files as commit-tracked artifacts. Why this matters: reviewable, resumable, auditable.
- **7.6** — Reviewing a plan before execution: 5-point check (right files, right order, complete code in steps, no placeholders, verification steps).
- **7.7** — Mid-plan replanning when reality bites. When to abandon, when to amend.
- **7.8** — Plan templates: bug fix, feature add, refactor, migration. One paragraph each.

- [ ] **Step 4: Fill 7.9–7.11 (edge cases + hands-on)**

Briefs:
- **7.9** — When to skip the plan: 1-file changes, trivial typos, exploratory work. Don't over-engineer.
- **7.10** — Plans across sessions: handoff via the plan file + commit history; resume mid-task.
- **7.11** — Mini-walkthrough: write the gitlog-report plan from the Ch 6 spec. Show task list with files-touched. (Cumulative — references prior chapter's walkthrough.)

- [ ] **Step 5: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 7 — Plan-Driven Execution"
```

---

## Task 11: Chapter 8 — Test-Driven Development with Agents

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 8.1–8.11)**

- [ ] **Step 2: Fill 8.1–8.4 (TDD with agents)**

Briefs:
- **8.1** — Why TDD matters more with agents: tests are the only thing keeping the agent honest. Without them, agents claim done while still broken.
- **8.2** — Red-green-refactor as control rod. Agents that can't run tests just write code; agents that can run tests iterate to passing.
- **8.3** — Writing the failing test first — yourself (best) or with the agent (acceptable but verify). The "test what the spec says" rule.
- **8.4** — "Stop until green" discipline. Don't refactor with reds; don't move on with reds.

- [ ] **Step 3: Fill 8.5–8.8 (specialized topics)**

Briefs:
- **8.5** — Golden tests: hand-curated input/output pairs. Anti-fragile to refactors.
- **8.6** — Snapshot tests: convenient but agents will happily update them to match wrong output. Treat snapshots as flagged, not authoritative.
- **8.7** — Anti-mocking: prefer integration tests against real dependencies (local DB, real filesystem). Mocks lie; integrations don't.
- **8.8** — Eval loops: test the *prompt*, not just the code. Examples of prompt evals (does the agent produce JSON? does it cite sources?).

- [ ] **Step 4: Fill 8.9–8.11 (signal + traps + hands-on)**

Briefs:
- **8.9** — Coverage as a verification signal — but a noisy one. 100% line coverage with no behavior tests is theatre.
- **8.10** — When the agent writes bad tests: tests that pass on broken code, tests that test the mock rather than the system. Signs.
- **8.11** — Mini-walkthrough: write the gitlog-report tests first — tests covering the acceptance criteria from the **6.11 spec**, structured along the task breakdown in the **7.11 plan**. Show 3–4 test cases (empty repo, single author, multi-author, date filter). Cumulative — the date-filter test is the one 9.11 will use to catch the injected bug.

- [ ] **Step 5: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 8 — TDD with Agents"
```

---

## Task 12: Chapter 9 — Debugging with Agents

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 9.1–9.11)**

- [ ] **Step 2: Fill 9.1–9.4 (the systematic loop)**

Briefs:
- **9.1** — The systematic debugging loop: reproduce → isolate → root cause → fix → verify. Map to superpowers:systematic-debugging skill.
- **9.2** — Reproduce reliably first. If you can't, your fix isn't a fix — it's a hope.
- **9.3** — Bugs agents fix wrong: cargo-culting, surface-symptom fixes that re-emerge, fixes that pass tests by mutating tests.
- **9.4** — Root cause vs patch: keeping the agent honest. The "why?" cascade until you hit invariants.

- [ ] **Step 3: Fill 9.5–9.8 (technique)**

Briefs:
- **9.5** — Reading the agent's hypotheses critically. When to push back.
- **9.6** — Bisecting with agent help: git bisect, binary search of inputs, narrowing.
- **9.7** — Evidence feeding: logs, traces, stack frames — what to paste, what to summarize, what to leave out.
- **9.8** — When to step in yourself: agent stuck in 3-iteration loop with no progress.

- [ ] **Step 4: Fill 9.9–9.11 (meta + hands-on)**

Briefs:
- **9.9** — Debugging the agent, not the code. When the prompt is the bug.
- **9.10** — The `superpowers:systematic-debugging` skill in Claude Code (no built-in `/debug` slash command — verify against current Claude Code commands); Debug mode in Kilo Code. Patterns and limits.
- **9.11** — Mini-walkthrough: inject an off-by-one bug into gitlog-report's date filter. The **date-filter test from 8.11 catches it** — that's the failing test that drives the debug loop. Walk the agent through reproduce → isolate → root cause → fix → verify. Cumulative.

- [ ] **Step 5: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 9 — Debugging with Agents"
```

---

## Task 13: Chapter 10 — Safety, Review & Verification *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 10.1–10.11)**

- [ ] **Step 2: Fill 10.1–10.4 (review layers)**

Briefs:
- **10.1** — Three review layers: pre-merge (you), code review (agent), security review (specialized agent or human). Each catches different things.
- **10.2** — Code review with agents: beyond rubber-stamping. Reviewing the agent's review.
- **10.3** — `/security-review` slash command (Claude Code): what it scans, what it misses. Calibration.
- **10.4** — `/ultrareview`: multi-agent deep review — different agents look for different categories. **Important context to call out: it runs in the cloud, is metered/paid, and is asynchronous (results return to inbox, not the terminal).** Use for important changes only.

- [ ] **Step 3: Fill 10.5–10.8 (verification + hallucinations)**

Briefs:
- **10.5** — Verification-before-completion (the skill). Don't claim done without running the thing. Show concrete pattern.
- **10.6** — Hallucinations in code: APIs that don't exist, imports that won't resolve. Signs. The Read-then-Edit verification pattern.
- **10.7** — The "looks right vs is right" gap. Code that compiles and reads well but doesn't actually work.
- **10.8** — Receiving review (from agent or human) gracefully. The superpowers:receiving-code-review skill. Verify findings, don't just implement.

- [ ] **Step 4: Fill 10.9–10.11 (self-review + gates + hands-on)**

Briefs:
- **10.9** — Self-review prompts: "list the ways this could be wrong"; "find a bug in this"; "what does this not handle". Show patterns.
- **10.10** — Sign-off gates: what blocks "done". Build passes; tests pass; manual smoke; security clean; reviewer happy.
- **10.11** — Mini-walkthrough: full review pass on the **debugged code from 9.11**. Run code review (inline subagent), run `/security-review` (local, synchronous), then dispatch `/ultrareview` (cloud, async — note the wait + inbox-results UX). Show one finding from each. Cumulative — the reviewed code from this step is what 11.11 will polish in parallel.

- [ ] **Step 5: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 10 — Safety, Review & Verification"
```

---

## Task 14: Chapter 11 — Multi-Agent & Parallel Workflows *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add chapter scaffold (subsections 11.1–11.11)**

- [ ] **Step 2: Fill 11.1–11.4 (subagents + isolation)**

Briefs:
- **11.1** — When one agent isn't enough: independent investigation, large context that would dilute main session, specialized roles.
- **11.2** — Subagents in Claude Code: the `Agent` tool, specialized types (typically lowercase identifiers — e.g., `general-purpose`, `code-reviewer`, `Explore` for read-only search, `Plan` for architectural design — **verify exact identifiers and casing against current Claude Code docs**). What each is for.
- **11.3** — Dispatching parallel subagents: rules of thumb — only when truly independent. Wire-up via single message with multiple Agent tool calls.
- **11.4** — Worktrees for isolation: `git worktree add`. Each parallel agent works in its own tree, no shared state.

- [ ] **Step 3: Fill 11.5–11.8 (recurring + orchestration)**

Briefs:
- **11.5** — `/loop` for recurring tasks. Polling external state (CI, deploy). Self-paced loops.
- **11.6** — `/schedule` for cron-style remote agents. Routines that run on their own.
- **11.7** — Agent-of-agents pattern: orchestrator decomposes, dispatches, integrates. Show signature pattern.
- **11.8** — Kilo Code Orchestrator mode: same idea in IDE-resident form. Multi-mode coordination.

- [ ] **Step 4: Fill 11.9–11.11 (limits + hands-on)**

Briefs:
- **11.9** — When parallel hurts: shared state, races (two agents editing same file), context fragmentation (each agent has half the picture).
- **11.10** — Token math: N parallel agents × M turns × tokens each. Multi-agent ≠ free — it's amortized only if work is genuinely parallel.
- **11.11** — Mini-walkthrough: gitlog-report polish pass. Building on Ch 10's reviewed code, dispatch 3 parallel subagents in 3 worktrees: (a) feature extension (e.g., add `--json` output flag), (b) additional test coverage (edge cases for date filters, large repos), (c) README + usage docs. Cumulative — does not re-implement Ch 7–8 work. Reference the `superpowers:dispatching-parallel-agents` and `superpowers:using-git-worktrees` skills.

- [ ] **Step 5: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 11 — Multi-Agent & Parallel"
```

---

## Task 15: Chapter 12 — Kilo Code Deep Dive *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

14 subsections.

- [ ] **Step 1: Add chapter scaffold (subsections 12.1–12.14)**

- [ ] **Step 2: Fill 12.1–12.4 (architecture + modes)**

Briefs:
- **12.1** — Kilo Code architecture: VS Code extension, Cline/Roo lineage, open-source. How it fits VS Code.
- **12.2** — The five modes — Architect, Code, Ask, Debug, Orchestrator. One paragraph each on what each is for.
- **12.3** — Why modes exist: prevent footguns. Architect can't run shell; Code can't define new specs; Ask is read-only. Constraint as feature.
- **12.4** — Picking a model per mode. Show `.kilocodemodes` snippet (or current config file name) routing Opus to Architect, Sonnet to Code, etc.

- [ ] **Step 3: Fill 12.5–12.8 (customization)**

Briefs:
- **12.5** — `.kilocodemodes` — defining custom modes (e.g., a "Reviewer" mode with specific model + tools).
- **12.6** — Custom rules: per-project rules file and global rules. What goes in each.
- **12.7** — Settings deep dive: auto-approve toggles per tool category, terminal integration, MCP server registration, telemetry.
- **12.8** — Memory and Context Bank: how Kilo persists info across sessions.

- [ ] **Step 4: Fill 12.9–12.12 (integration + cost + scale)**

Briefs:
- **12.9** — MCP integration in Kilo — adding a server via settings, calling its tools across modes.
- **12.10** — Commit message and PR features.
- **12.11** — Cost dashboard and tracking — read the per-session cost, identify hotspots.
- **12.12** — Multi-repo / monorepo setups — workspace folders, per-folder rules.

- [ ] **Step 5: Fill 12.13–12.14 (shortcuts + hands-on)**

Briefs:
- **12.13** — Power-user shortcuts: hotkeys, common slash-equivalents, mode-quick-switch.
- **12.14** — Hands-on: configure Kilo Code for your daily workflow. Walk through: install → BYO key → set up modes → pick models → add MCP → commit `.kilocodemodes`.

- [ ] **Step 6: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 12 — Kilo Code Deep Dive"
```

---

## Task 16: Chapter 13 — Claude Code Deep Dive *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

14 subsections.

- [ ] **Step 1: Add chapter scaffold (subsections 13.1–13.14)**

- [ ] **Step 2: Fill 13.1–13.4 (architecture + memory)**

Briefs:
- **13.1** — Claude Code architecture: terminal CLI, headless mode, IDE extensions (VS Code, JetBrains). Where it lives.
- **13.2** — CLAUDE.md hierarchy: `~/.claude/CLAUDE.md` (global) → project root → subdirectory. What goes in each scope.
- **13.3** — Slash commands: built-ins (`/clear`, `/compact`, `/init`, `/review`, `/security-review`, etc.) and custom commands. Where custom commands live.
- **13.4** — Skills: skill files in `~/.claude/skills/<name>/SKILL.md`, frontmatter, body. The `Skill` tool. Auto-discovery.

- [ ] **Step 3: Fill 13.5–13.8 (plugins + hooks + subagents)**

Briefs:
- **13.5** — Plugins and the plugin marketplace. Installing, listing, removing. Plugin = bundle of skills/commands/hooks.
- **13.6** — Hooks lifecycle: `SessionStart`, `PreToolUse`, `PostToolUse`, `Stop`, etc. Use cases per event. Example hook in `settings.json`.
- **13.7** — Sub-agents: the `Agent` tool, specialized types (`general-purpose`, `Explore`, `Plan`, `code-reviewer`, etc. — **verify exact identifiers and casing against current Claude Code docs**). When to use which.
- **13.8** — Plan mode workflow: enter, plan, approve, execute. The approval gate.

- [ ] **Step 4: Fill 13.9–13.12 (personalization)**

Briefs:
- **13.9** — Output styles: changing the response tone/format via the styles system.
- **13.10** — Fast mode: faster Opus output, when to toggle, what changes.
- **13.11** — Scheduling primitives: `ScheduleWakeup`, `/loop`, `/schedule`. What each is for and where they overlap.
- **13.12** — Settings and permissions: `.claude/settings.json` schema. Allowlist, denylist, hooks, env vars.

- [ ] **Step 5: Fill 13.13–13.14 (memory + hands-on)**

Briefs:
- **13.13** — Auto-memory system: `MEMORY.md` index + per-memory files. Types (user, feedback, project, reference). When the agent saves automatically.
- **13.14** — Hands-on: configure Claude Code for your daily workflow. CLAUDE.md, settings.json permissions, install a plugin, write a custom slash command, verify it fires.

- [ ] **Step 6: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 13 — Claude Code Deep Dive"
```

---

## Task 17: Chapter 14 — Gemini CLI Deep Dive *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

15 subsections.

**Note on freshness:** Gemini CLI is moving fast. Before writing, run a quick sanity check by visiting `https://github.com/google-gemini/gemini-cli` (or the current canonical repo) and the official docs to confirm command names and feature scope as of writing time. Note any deltas from the Jan 2026 snapshot in this plan.

- [ ] **Step 1: Add chapter scaffold (subsections 14.1–14.15)**

- [ ] **Step 2: Fill 14.1–14.4 (architecture + models)**

Briefs:
- **14.1** — Gemini CLI architecture: terminal, open-source (Apache 2.0), Google-backed. Position vs Claude Code.
- **14.2** — Model story: Gemini 3 Pro/Flash defaults, model routing in `settings.json`, ability to plug other models via OpenAI-compat or direct.
- **14.3** — Authentication paths: personal Google account (free tier), API key (Gemini API), Vertex AI (enterprise). Free-tier limits — 1M context, ~1000 req/day (verify current numbers).
- **14.4** — GEMINI.md hierarchy: global / project / subdirectory. CLAUDE.md cousin. Show example file.

- [ ] **Step 3: Fill 14.5–14.8 (skills + extensions + hooks)**

Briefs:
- **14.5** — Slash commands: built-ins (verify current list) and how to add custom.
- **14.6** — Skills in Gemini CLI: how `activate_skill` works, skill metadata loaded at session start, activation on demand. Mirror to Claude Code skills.
- **14.7** — Extensions: install (likely `gemini extensions install --source <git-url>` for remote or `--path` for local — **verify exact flag syntax against current Gemini CLI docs before writing**), discover, share. The plugin equivalent.
- **14.8** — Hooks: lifecycle events supported by Gemini CLI (verify current list — likely narrower than Claude Code).

- [ ] **Step 4: Fill 14.9–14.12 (tools + MCP + settings)**

Briefs:
- **14.9** — Built-in tools: Read, Write, Shell, GoogleSearch, WebFetch, MemoryTool. One paragraph each on what's available.
- **14.10** — MCP integration in Gemini CLI: registering servers, calling tools. Differences from Claude Code's MCP support.
- **14.11** — Settings: `settings.json` schema, permissions, the YOLO mode question (auto-approve everything).
- **14.12** — Memory and context tools — how Gemini CLI persists state.

- [ ] **Step 5: Fill 14.13–14.15 (context + hands-on)**

Briefs:
- **14.13** — Working with 1M context: what changes when context is no longer scarce. Anti-patterns (fill it just because you can) and patterns (keeping full repo state in working memory).
- **14.14** — Hands-on: configure Gemini CLI for your daily workflow. Install, auth via personal Google, GEMINI.md, install one extension, register one MCP server.
- **14.15** — See-also (→ Ch 2 context, → Ch 4 cost & free tier, → Ch 15 Superpowers in Gemini CLI).

- [ ] **Step 6: Render and verify**

- [ ] **Step 7: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 14 — Gemini CLI Deep Dive"
```

---

## Task 18: Chapter 15 — The Superpowers Methodology *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

15 subsections after the trim discussed in brainstorming.

- [ ] **Step 1: Add chapter scaffold (subsections 15.1–15.15)**

- [ ] **Step 2: Fill 15.1–15.4 (concept + three implementations)**

Briefs:
- **15.1** — Why methodology bundles exist: codify how you want the agent to work. Tool-agnostic at heart.
- **15.2** — The Superpowers methodology: brainstorm → spec → plan → execute → TDD → verify. The portable patterns.
- **15.3** — The library tour: brainstorming, writing-plans, executing-plans, subagent-driven-development, test-driven-development, systematic-debugging, verification-before-completion, requesting-code-review, receiving-code-review, finishing-a-development-branch, writing-skills. What each enforces.
- **15.4** — Three implementations, one idea: Claude Code skills, Gemini CLI skills, Kilo Code modes/rules. Where each shines.

- [ ] **Step 3: Fill 15.5–15.8 (Claude Code primary)**

Briefs:
- **15.5** — The Claude Code plugin: installation via the `/plugin marketplace add obra/superpowers-marketplace` → `/plugin` interactive flow (**verify exact commands against current Claude Code docs**), then the `using-superpowers` meta-skill auto-invokes via the `SessionStart` hook on every session.
- **15.6** — The Claude Code skill machinery: format (frontmatter + body), the `Skill` tool, skill discovery from `~/.claude/skills/` and plugin dirs.
- **15.7** — Rigid vs flexible skills: when to enforce (TDD, debugging), when to suggest (patterns). The skill itself signals this.
- **15.8** — Anatomy of a great skill: name (kebab-case), description (when to use), body (instructions for the agent), references (linked sub-skills).

- [ ] **Step 4: Fill 15.9–15.11 (Kilo + Gemini adaptations)**

Briefs:
- **15.9** — Adapting to Kilo Code: a custom mode in `.kilocodemodes` whose system prompt is the skill body; pair with model selection.
- **15.10** — Adapting to Gemini CLI: GEMINI.md or skill file in Gemini's skills directory; `activate_skill` semantics.
- **15.11** — What you give up outside Claude Code: auto-invocation via SessionStart. Work around: explicit mode selection (Kilo), explicit `activate_skill` calls (Gemini), or session-start prompt that lists the methodology.

- [ ] **Step 5: Fill 15.12–15.15 (write + ship + hands-on)**

Briefs:
- **15.12** — Writing your first skill: pick a daily friction, write the SKILL.md, install it locally, watch it fire. Walk through with a real example (e.g., "always commit at end of task").
- **15.13** — Skill/mode composition: calling skills from skills via `Skill` tool; chaining modes in Kilo Orchestrator.
- **15.14** — Testing skills (does it fire?), anti-patterns (skills that don't trigger, skills that overrule user instructions).
- **15.15** — Hands-on: ship a methodology bundle for your team. Pick one workflow (e.g., your team's PR-creation flow), encode it as a skill (Claude Code), a mode (Kilo Code), and a session-start prompt (Gemini CLI). Distribute.

- [ ] **Step 6: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 15 — Superpowers Methodology"
```

---

## Task 19: Chapter 16 — MCP Servers — Using and Building *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

14 subsections.

- [ ] **Step 1: Add chapter scaffold (subsections 16.1–16.14)**

- [ ] **Step 2: Fill 16.1–16.4 (concept + primitives + transports + tour)**

Briefs:
- **16.1** — MCP in plain terms: open protocol for connecting LLMs to external tools/data. Why it matters: ecosystem leverage.
- **16.2** — Three primitives: tools (callable), resources (referenceable), prompts (guided workflows). Diagram each.
- **16.3** — Transport: stdio (local, simplest) vs HTTP/SSE (remote, multi-client). Tradeoffs.
- **16.4** — Existing servers tour: Atlassian, Slack, Box, Figma, GitHub. One paragraph each — what it gives the agent.

- [ ] **Step 3: Fill 16.5–16.8 (using + authoring scaffold)**

Briefs:
- **16.5** — Adding an MCP server to Kilo Code and Claude Code. Show settings snippet for each (stdio config).
- **16.6** — Authoring your own MCP server. Language picks: Python (`mcp` SDK) or TypeScript (`@modelcontextprotocol/sdk`). Pros/cons.
- **16.7** — Server skeleton: handlers for `list_tools`, `call_tool`, `list_resources`, `read_resource`. Show a Python code skeleton as the canonical example (the hands-on at 16.14 also uses Python). Acknowledge the TypeScript equivalent has the same handler shape via `@modelcontextprotocol/sdk` — link to the official TS quick-start rather than duplicating the snippet.
- **16.8** — Designing tools agents use well: clear descriptions, narrow parameters, idempotent where possible. Anti-pattern: kitchen-sink tools.

- [ ] **Step 4: Fill 16.9–16.12 (authoring deep)**

Briefs:
- **16.9** — Authoring resources: when a resource fits better than a tool (read-only data, large content the agent might reference).
- **16.10** — Authoring prompts: pre-built guided workflows the agent or user can invoke.
- **16.11** — Auth and secrets: env vars, OAuth, where each fits.
- **16.12** — Testing your MCP server: unit tests for handlers, integration tests via the MCP inspector tool.

- [ ] **Step 5: Fill 16.13–16.14 (distribute + hands-on)**

Briefs:
- **16.13** — Publishing and distribution: pypi/npm, GitHub releases, the MCP registry, sharing with your team.
- **16.14** — Hands-on: build a tiny `local-notes` MCP server. Tools: `add_note`, `list_notes`, `search_notes`. SQLite-backed. Show full code (~50 lines Python). Register with Claude Code, invoke, verify.

- [ ] **Step 6: See-also + render + commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 16 — MCP Servers"
```

---

## Task 20: Chapter 17 — Worked Example: Ship gitlog-report End-to-End *(featured)*

**Files:**
- Modify: `hero-to-architect-guide.html`

15 subsections — the big stitch. References cumulative walkthroughs from Ch 6–11.

- [ ] **Step 1: Add chapter scaffold (subsections 17.1–17.15)**

- [ ] **Step 2: Fill 17.1–17.2 (brief + framing)**

Briefs:
- **17.1** — The brief: `gitlog-report` CLI. Acceptance criteria (full list, ~8 bullets): reads git log; per-author summary; commits, lines, top files; JSON + markdown output; date range filter; handles empty repo; tested.
- **17.2** — Why multiple passes: different tools surface different things. Single-tool runs hide the trade-offs.

- [ ] **Step 3: Fill 17.3–17.7 (Pass 1 — Kilo Code)**

Briefs:
- **17.3** — Pass 1 brainstorm + spec + plan (Kilo Architect mode). Show: enter Architect, prompt with brief, agent produces OpenSpec proposal, switch to Code mode with plan.
- **17.4** — Pass 1 TDD in Code mode. Write the failing tests file (from Ch 8 walkthrough), run, watch agent implement.
- **17.5** — Pass 1 debug. Inject the bug from Ch 9 walkthrough (date filter off-by-one). Switch to Debug mode. Walk the diagnosis.
- **17.6** — Pass 1 review pass. Architect or Reviewer mode. Run through the review checklist from Ch 10.
- **17.7** — Pass 1 ship. Open PR, fill template, merge.

- [ ] **Step 4: Fill 17.8–17.11 (Pass 2 — Claude Code)**

Briefs:
- **17.8** — Pass 2 brainstorm + spec + plan via Superpowers + SpecKit + plan mode. Show: `/brainstorming` flow, `/specify` for the spec, plan mode for the plan.
- **17.9** — Pass 2 execute via parallel subagents. 3 worktrees, 3 agents: feature, tests, docs. Single dispatching message.
- **17.10** — Pass 2 verification: `/security-review`, `/ultrareview`, manual smoke. Hits from each.
- **17.11** — Pass 2 ship. PR via `gh pr create`.

- [ ] **Step 5: Fill 17.12 — Pass 3 highlights (Gemini CLI)**

Brief:
- **17.12** — Pass 3 highlights. Run the same project in Gemini CLI. Focus on what's *different*: free-tier cost, 1M context lets the agent hold the whole project in working memory at once, extensions used. Don't repeat the full walkthrough.

- [ ] **Step 6: Fill 17.13–17.15 (compare + lessons)**

Briefs:
- **17.13** — Side-by-side: cost (tokens, dollars), wall-clock time, ease, where each shone, where each tripped. Table.
- **17.14** — Lessons learned: ~6 patterns to repeat, ~4 antipatterns to avoid. Concrete bullets.
- **17.15** — See-also (→ all prior methodology chapters; → Part 5 reference).

- [ ] **Step 7: Render and verify**

This chapter is long. Verify the in-chapter anchors (if any sub-anchors were added) all jump correctly.

- [ ] **Step 8: Commit**

```bash
git commit -am "docs(hero-to-architect): write Ch 17 — Worked Example end-to-end"
```

---

## Task 21: Reference — Decision Frameworks

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add section scaffold**

```html
<section id="ref-decisions" class="section">
  <h1>Decision Frameworks</h1>
  <p class="lede">Choose-the-right-thing matrices distilled from the rest of the guide. Each links back to the chapter that explains the underlying reasoning.</p>
  <!-- Matrices below -->
</section>
```

- [ ] **Step 2: Add matrix — Which tool**

Rows: Kilo Code, Claude Code, Gemini CLI, Antigravity, Cursor, Aider. Cols: Surface (IDE/terminal), Free tier?, MCP support, Plugin ecosystem, Best for. Link to Ch 5/12/13/14.

- [ ] **Step 3: Add matrix — Which model for which task**

Rows: Quick edit, multi-file refactor, deep architecture, code review, security scan, doc writing, agentic loop driver. Cols: Recommended primary model + thinking mode + caching note. Link to Ch 4.

- [ ] **Step 4: Add matrix — Which mode/skill for which task**

Rows: task types. Cols: Kilo mode | Claude skill | Gemini skill. Link to Ch 12/13/14/15.

- [ ] **Step 5: Add matrix — OpenSpec vs SpecKit**

Rows: workflow style, change tracking, file structure, tool support, learning curve. Cols: OpenSpec | SpecKit | When to pick. Link to Ch 6.

- [ ] **Step 6: Add 3 more compact decision charts**

- When to plan vs skip (one paragraph + flowchart)
- When to parallelize vs serialize (one paragraph + checklist)
- When to engage thinking mode (cost-benefit one paragraph)

- [ ] **Step 7: Render and verify**

- [ ] **Step 8: Commit**

```bash
git commit -am "docs(hero-to-architect): add Reference — Decision Frameworks"
```

---

## Task 22: Reference — Cheat Sheets

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add section scaffold**

```html
<section id="ref-cheats" class="section">
  <h1>Cheat Sheets</h1>
</section>
```

- [ ] **Step 2: Kilo Code cheat sheet**

Command/mode/shortcut reference: install command, mode switch shortcuts, common slash equivalents, `.kilocodemodes` snippet template.

- [ ] **Step 3: Claude Code cheat sheet**

Slash commands (built-in, frequently used), hook lifecycle events, skill frontmatter template, `.claude/settings.json` template.

- [ ] **Step 4: Gemini CLI cheat sheet**

Slash commands, `activate_skill` example, extension install command, `GEMINI.md` template, free-tier limits at-a-glance.

- [ ] **Step 5: MCP cheat sheet**

Popular servers with one-line install snippets each. Settings.json snippet templates for stdio config.

- [ ] **Step 6: OpenSpec cheat sheet**

Command reference (verify current commands), proposal file template, delta spec template.

- [ ] **Step 7: SpecKit cheat sheet**

`/specify`, `/plan`, `/tasks`, `/implement` — what each produces, when each fires.

- [ ] **Step 8: Superpowers skill library at-a-glance**

Tight one-liner per skill (cheat-sheet form — Ch 15.3 has the full library tour, this is just a quick-reference table). Cross-link the section heading to `#ch15-superpowers`. Use the same skill order as Ch 15.3 to avoid reader confusion when cross-referencing. Skills: brainstorming, writing-plans, executing-plans, subagent-driven-development, test-driven-development, systematic-debugging, verification-before-completion, requesting-code-review, receiving-code-review, finishing-a-development-branch, writing-skills.

- [ ] **Step 9: Render and verify**

- [ ] **Step 10: Commit**

```bash
git commit -am "docs(hero-to-architect): add Reference — Cheat Sheets"
```

---

## Task 23: Reference — Glossary A–Z

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add section scaffold**

```html
<section id="ref-glossary" class="section">
  <h1>Glossary A–Z</h1>
  <p class="lede">Extends the v1 glossary. Entries marked <span class="v1-marker">v1</span> are covered there; this list adds advanced-tier terms.</p>
</section>
```

- [ ] **Step 2: Add glossary entries (alphabetical)**

Entries (one paragraph each):

Sort case-insensitively, treating a leading `/` as if absent for ordering. Pre-sorted seed list:

`activate_skill` · `Agent` tool · agent loop · auto-approve · blast radius · BYO model · cache hit rate · CLAUDE.md · Cline · Context Bank · context compaction · Continue.dev · custom mode · extensions (Gemini) · fast mode · GEMINI.md · golden test · headless mode · hooks · HTTP/SSE transport · idempotency · Junie · Kilo custom modes (verify filename) · `/loop` · MCP · MCP inspector · MCP registry · MCP resource · MCP tool · memory file · OpenSpec · Orchestrator mode · output style · plan mode · plugin · prompt cache · `/schedule` · `ScheduleWakeup` · SessionStart hook · skill · slash command · SpecKit · spec-driven dev · stdio transport · sub-agent · Superpowers · thinking mode · verification-before-completion · Vertex AI · worktree · YOLO mode.

*(Removed: `tool-alt` — that's a CSS class, not a methodology term readers will search for.)*

For each entry: 1–3 sentences + a "→ Ch X" link to where it's covered in depth.

- [ ] **Step 3: Render and verify**

Check alphabetization, internal anchor links, mobile layout.

- [ ] **Step 4: Commit**

```bash
git commit -am "docs(hero-to-architect): add Reference — Glossary A-Z"
```

---

## Task 24: Reference — Further Reading

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: Add section scaffold**

```html
<section id="ref-reading" class="section">
  <h1>Further Reading</h1>
</section>
```

- [ ] **Step 2: Add canonical docs grid**

Card-grid of links: Claude Code docs, Kilo Code docs, Gemini CLI docs, MCP spec, OpenSpec project, SpecKit project, Superpowers project, Antigravity docs. Each card: name, URL, what's there, freshness note.

- [ ] **Step 3: Add curated reading list**

Bullets: papers, talks, community resources, blog series worth reading. Group by topic (agentic patterns, spec-driven dev, prompting, evals).

- [ ] **Step 4: Add "If you liked this guide"**

Cross-links to the v1 guide and the networking guides in this repo. Short paragraph framing the wider series.

- [ ] **Step 5: Render and verify**

- [ ] **Step 6: Commit**

```bash
git commit -am "docs(hero-to-architect): add Reference — Further Reading"
```

---

## Task 25: Cross-link verification pass

**Files:**
- Modify: `hero-to-architect-guide.html`

- [ ] **Step 1: List all referenced ids (sidebar + see-also links)**

```bash
grep -oE "go\('[a-z0-9-]+'\)" /Users/divakaran/arrcus_workspace/guides/hero-to-architect-guide.html \
  | sed -E "s/go\('([a-z0-9-]+)'\)/\1/" | sort -u > /tmp/refs.txt
cat /tmp/refs.txt
```

Expected: every id referenced should also appear as a `<section id="...">` (next step).

- [ ] **Step 2: Cross-check refs against actual `<section id>` targets**

```bash
grep -oE '<section[^>]*id="[a-z0-9-]+"' /Users/divakaran/arrcus_workspace/guides/hero-to-architect-guide.html \
  | sed -E 's/.*id="([^"]+)"/\1/' | sort -u > /tmp/section-ids.txt
comm -23 /tmp/refs.txt /tmp/section-ids.txt   # refs with no matching section = orphan link
comm -13 /tmp/refs.txt /tmp/section-ids.txt   # sections with no incoming link (acceptable for some, e.g. welcome)
```

Expected: the first `comm` (orphans) must be empty. The second `comm` may contain some sections (e.g., front-matter); only worry about chapters/reference pages with no incoming links.

- [ ] **Step 3: Verify each chapter's "See also" grid**

Walk Ch 1–17 manually in the browser. Click each see-also link. Confirm jump target loads correct chapter.

- [ ] **Step 4: Commit (only if fixes made)**

```bash
git commit -am "fix(hero-to-architect): repair cross-link issues"
```

---

## Task 26: Visual review pass

**Files:**
- Modify: `hero-to-architect-guide.html` (only if fixes needed)

- [ ] **Step 1: Open in default browser, walk every section**

Check each chapter renders without overflow, code blocks scroll horizontally rather than break the layout, tables don't blow out on narrow widths.

- [ ] **Step 2: Toggle dark/light mode on every section**

Look for unreadable color combos (especially on info boxes, tool-alt callouts, walkthrough boxes, card grids).

- [ ] **Step 3: Test on narrow width**

Resize browser to ~480px. Check: sidebar collapse behavior (matches v1?), card grids wrap properly, code blocks don't overflow.

- [ ] **Step 4: Console check**

Load the page in a fresh tab. Open DevTools → Console. Click 3 random sidebar entries spanning different Parts. Confirm `console.error` count remains 0 throughout. If any warnings appear, decide whether to fix or accept.

- [ ] **Step 5: Fix and commit (if needed)**

```bash
git commit -am "fix(hero-to-architect): visual polish"
```

---

## Task 27: README update + final commit

**Files:**
- Modify: `/Users/divakaran/arrcus_workspace/guides/README.md`

- [ ] **Step 1: Add row to the guides table**

In README.md, add to the Guides table:

```markdown
| Gen AI: Hero to Architect | [hero-to-architect-guide.html](hero-to-architect-guide.html) | Advanced companion to Zero to Hero — agentic coding loop, context engineering, OpenSpec, SpecKit, Superpowers, Kilo Code, Claude Code, Gemini CLI, MCP, multi-agent workflows, and an end-to-end worked example shipping a CLI tool in three tools |
```

- [ ] **Step 2: Update `IDEAS.md` if the guide is listed there**

```bash
grep -l "Hero to Architect" /Users/divakaran/arrcus_workspace/guides/IDEAS.md && echo "FOUND — edit the row's Status column to 'Done'" || echo "NOT LISTED — skip"
```

If found, change that row's Status to `Done`. If not listed, no action needed.

- [ ] **Step 3: Verify README renders correctly**

```bash
cat /Users/divakaran/arrcus_workspace/guides/README.md
```

- [ ] **Step 4: Final commit**

```bash
git add README.md
git commit -m "docs: add Hero to Architect guide to README"
```

- [ ] **Step 5: Optional — open the finished guide one more time**

Click through random chapters end-to-end. If anything feels rough, file a follow-up issue rather than fix in this PR.

---

## Self-Review Notes (writer to writer)

**Spec coverage:** Every chapter in spec §8 has a task here. Every cross-cutting thread (mini-walkthroughs, see-also grids, tool-alt callouts) has a CSS hook (Task 1 Step 5) and is referenced in every relevant chapter task. Reference pages mapped 1:1 to spec §8 Part 5. Open items in spec §14 are addressed inline in Task 1 (accent color), Task 25 (sidebar grouping), Task 17 (Gemini CLI freshness check), and Task 8.9 (Antigravity card depth).

**Placeholder scan:** Subsection content briefs describe what to write rather than reproducing the prose itself. This is a deliberate adaptation for a long-form content project — reproducing 200+ subsections of finished prose inside a plan would balloon the plan to the size of the artifact. Each brief is concrete enough that a writer with the spec + this plan + the v1 guide can produce the content without further clarification.

**Type consistency:** Section ids consistent: `ch1-loop`, `ch2-context`, ..., `ch17-worked`, `ref-decisions`, `ref-cheats`, `ref-glossary`, `ref-reading`. CSS class names consistent: `tool-alt`, `walkthrough`, `card-grid`, `tool-card`, `see-also`. CSS variable names from v1 preserved.

**Bite-size adaptation:** For a 200K+ word content project, strict "2–5 minute steps with full code shown" is not appropriate. Tasks are sized to one logical commit (~one chapter), and steps within tasks are sized to one logical content unit. The granularity here serves resumability and review — each commit produces a runnable, viewable artifact.
