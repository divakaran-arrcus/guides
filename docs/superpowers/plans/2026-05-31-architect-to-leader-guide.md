# Architect to Leader Guide Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. The `guide-builder` skill should also be loaded — it has the canonical component vocabulary for this repo's HTML guides.

**Goal:** Build a single-page HTML guide `gen-ai-architect-to-leader-guide.html` — "Gen AI for Developers: Architect to Leader", Part 3 of the series — on scaling agentic coding across an organization. 16 chapters across a 5-stage maturity model + reference, narrated through the "Meridian" case study, tool-agnostic.

**Architecture:** One self-contained HTML file. Sidebar-driven SPA pattern (sections with `display:none`, JS `go(id)` activates one). Reuses Part 2's chrome (fonts, color tokens, info boxes, comparison grids, step blocks, tables, `.tool-alt`, `.see-also`). Each chapter = one `<section>` whose id matches its sidebar nav-item. Two new components: a per-chapter lens tag and a Meridian case-study scene box.

**Tech Stack:** Hand-authored HTML5 + embedded CSS + vanilla JS. No build step. Opens directly in a browser. Source spec: `docs/superpowers/specs/2026-05-31-architect-to-leader-guide-design.md`.

---

## File Structure

- Create: `/Users/divakaran/arrcus_workspace/guides/gen-ai-architect-to-leader-guide.html`
- Modify: `/Users/divakaran/arrcus_workspace/guides/README.md` (add row to Gen AI table — late task)
- Modify: `/Users/divakaran/arrcus_workspace/guides/gen-ai-zero-to-hero-guide.html` (add forward link to Part 3 — late task)
- Modify: `/Users/divakaran/arrcus_workspace/guides/gen-ai-hero-to-architect-guide.html` (add forward link to Part 3 — late task)

No other files. Embedded CSS + JS in the HTML (matches the repo's other guides).

---

## Conventions used in this plan

**Each "chapter task" follows the same pattern:**
1. Add chapter `<section id="ch.." class="section">` scaffold: `<h1>`, a lens tag (`<span class="lens-tag ..">`), a "What you'll learn" preamble, and `<h2>` placeholders for each subsection. **The `class="section"` is required** — the SPA hides every `.section` by default (`display:none`) and `go()` shows one via `.section.active`; a `<section>` without the class renders always-visible and is never hidden on navigation. Write the opening tag as `id="…" class="section"` on a single line so the Task 22 integrity check (which matches that exact attribute order) sees it. (Every section in this guide carries `class="section"`; `welcome` keeps `class="section active"`.)
2. Fill subsection content in groups of 2–4 (one step per group). Each subsection = 2–4 paragraphs / point-wise lists, following the per-chapter brief in the task.
3. Add the Meridian case-study scene box (`<div class="case">`) where the task specifies one.
4. Add the "See also →" cross-link grid (`<div class="see-also">`) with the targets listed in the task.
5. Render in a browser, navigate to the chapter, verify layout + dark/light toggle + no console errors.
6. Commit: `docs(architect-to-leader): write Ch N — <title>`.

**Content style (match Parts 1-2):** point-wise `<li>` under short mini-headings (`<h4 class="ph">`), declarative second-person, short sentences, no hedging on principles. Volatile facts (pricing, model names, tool commands) carry a "verify current" note. Prefer capability language over version numbers; where a model is named, use the series convention (Claude Opus/Sonnet 4.7, GPT-5, Gemini 3).

**Tool-agnostic rule:** every concrete artifact (config files, commands, CI snippets) is shown for **at least two** tools — Claude Code plus one of Cursor / Kilo / Copilot — using the `.tool-alt` aside for the secondary example. No example assumes a single vendor.

**Lens tag component:** each chapter declares its primary lens(es) with a small pill: `<span class="lens-tag people">People &amp; Process</span>`, `… platform">Platform &amp; Infrastructure</span>`, `… governance">Governance &amp; Risk</span>`. CSS added in Task 1.

**Meridian scene component:** a `<div class="case">` with a `<div class="case-label">Meridian</div>` header and 3–6 short bullets advancing the company's story. Cumulative — each scene builds on the prior chapter's. CSS added in Task 1.

**Commit convention:** scaffolding/nav tasks use `feat(architect-to-leader): …`; content tasks use `docs(architect-to-leader): …`; fix-ups use `fix(architect-to-leader): …`.

**Verification convention (per render step):** the full sidebar (all 23 `go()` targets) is built once in Task 2, but sections are added one task at a time — so a whole-file nav↔section parity check would report every *not-yet-built* section as unlinked until the build finishes. That full parity check therefore lives in **Task 22**. Per task, run only the div-balance check and confirm the id(s) you built *this task* now resolve:
```bash
cd /Users/divakaran/arrcus_workspace/guides
f=gen-ai-architect-to-leader-guide.html
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
for id in <ids-built-this-task>; do
  grep -q "id=\"$id\" class=\"section" "$f" && echo "ok $id" || echo "MISSING $id"
done
```
Expected: `div N/N` (balanced), and `ok <id>` for every section built this task. Not-yet-built sections remaining unlinked is expected until Task 22 runs the full parity check.

---

## Task 1: Scaffold the HTML file from Part 2's chrome

**Files:**
- Create: `/Users/divakaran/arrcus_workspace/guides/gen-ai-architect-to-leader-guide.html`
- Read: `/Users/divakaran/arrcus_workspace/guides/gen-ai-hero-to-architect-guide.html` (source pattern — latest chrome)

- [ ] **Step 1: Copy Part 2 as the starting point**

```bash
cp /Users/divakaran/arrcus_workspace/guides/gen-ai-hero-to-architect-guide.html \
   /Users/divakaran/arrcus_workspace/guides/gen-ai-architect-to-leader-guide.html
```

- [ ] **Step 2: Update `<title>` and sidebar brand**

Edit the new file:
- `<title>Gen AI for Developers: Architect to Leader</title>`
- Sidebar brand `<h1>` → `Architect to Leader`
- Sidebar tagline `<p>` → `Part 3 — scaling agentic coding across an organization`

- [ ] **Step 3: Set the Part 3 accent token**

Part 2 defines `--accent` in both the default `:root` and the `body.light` block. Change both to teal so the reader can tell Part 3 apart at a glance:
- default `:root`: `--accent: #2dd4bf;` (teal)
- `body.light` override: `--accent: #0d9488;` (deeper teal for contrast)

- [ ] **Step 4: Add the two new components to the `<style>` block**

Add after the existing `.info` rules:
```css
.lens-tag { display:inline-block; font-size:11px; font-weight:600; letter-spacing:.02em;
  padding:2px 9px; border-radius:999px; margin:0 6px 0 0; border:1px solid var(--border); }
.lens-tag.people     { color:var(--green);  border-color:var(--green); }
.lens-tag.platform   { color:var(--blue);   border-color:var(--blue); }
.lens-tag.governance { color:var(--orange); border-color:var(--orange); }
.case { border-left:3px solid var(--accent); background:color-mix(in srgb, var(--accent) 6%, transparent);
  padding:14px 18px; border-radius:0 8px 8px 0; margin:18px 0; }
.case-label { font-size:11px; font-weight:700; letter-spacing:.08em; text-transform:uppercase;
  color:var(--accent); margin-bottom:6px; }
```

- [ ] **Step 5: Reduce `<main>` to a welcome stub**

Goal: keep `<head>`/`<style>`, the `<nav class="sidebar">` shell (nav-groups replaced in Task 2), `<main>` open/close, the `welcome` section (stub to retitle in Task 3), and the `<script>` block. Remove every other `<section>…</section>`.

Do it mechanically, not by eyeballing 8,600 lines:
1. Map the boundaries: `grep -n '<section\|</section>\|</main>' gen-ai-architect-to-leader-guide.html`
2. The first `<section id="welcome"` … its matching `</section>` is the stub — keep it. Everything from the *next* `<section` up to (but not including) `</main>` is content to remove.
3. Delete those blocks **bottom-up** (highest line numbers first) so earlier line numbers stay valid as you go. Each delete spans a full `<section …>` … `</section>` pair — never a partial block.
4. After each delete (or at the end), run the div-balance check: `o=$(grep -o "<div" gen-ai-architect-to-leader-guide.html|wc -l); c=$(grep -o "</div>" gen-ai-architect-to-leader-guide.html|wc -l); echo "div $o/$c"`. It must stay balanced. If it doesn't, you cut a partial block — revert that delete and redo on full `<section>…</section>` boundaries.

(If a clean strip proves fiddly, the equivalent fallback is to author `<main>` fresh: keep the copied `<head>`/`<nav>`/`<script>` and replace the entire `<main>…</main>` body with just the `welcome` stub section.)

- [ ] **Step 6: Verify the shell loads**

```bash
cd /Users/divakaran/arrcus_workspace/guides
grep -c "<section" gen-ai-architect-to-leader-guide.html   # expect 1 (welcome stub)
o=$(grep -o "<div" gen-ai-architect-to-leader-guide.html|wc -l); c=$(grep -o "</div>" gen-ai-architect-to-leader-guide.html|wc -l); echo "div $o/$c"
```
Expected: 1 section; balanced divs. Open in a browser — page loads, sidebar + theme toggle render, no console errors.

- [ ] **Step 7: Commit**

```bash
git add gen-ai-architect-to-leader-guide.html
git commit -m "feat(architect-to-leader): scaffold HTML shell + accent + lens/case components"
```

---

## Task 2: Build the sidebar navigation

**Files:** Modify `gen-ai-architect-to-leader-guide.html` (the `<nav class="sidebar">` nav-groups).

- [ ] **Step 1: Replace the nav-groups with the Part 3 structure**

Replace all `<div class="nav-group">…</div>` blocks (between the theme-toggle and `</nav>`) with these groups and buttons (each `onclick="go('id')"` must match a section id created later):

- **Front Matter:** `welcome` → "Welcome", `map` → "The Map"
- **Stage 0→1 · Making the Case:** `ch1-scale`, `ch2-case`, `ch3-pilot`
- **Stage 1→2 · People & Process:** `ch4-standards`, `ch5-enablement`, `ch6-change`, `ch7-metrics`
- **Stage 2→3 · Platform:** `ch8-config`, `ch9-registry`, `ch10-cicd`, `ch11-cost`
- **Stage 3→4 · Governance & Risk:** `ch12-security`, `ch13-gates`, `ch14-incident`, `ch15-autonomy`
- **Putting It Together:** `ch16-rollout`
- **Reference:** `ref-scorecard`, `ref-checklists`, `ref-metrics`, `ref-glossary`, `ref-reading`

Button labels: use the chapter titles from the spec (e.g. `1. Why Individual Mastery Doesn't Scale`, `8. Golden Configuration as Code`, etc.).

- [ ] **Step 2: Commit**

```bash
git add gen-ai-architect-to-leader-guide.html
git commit -m "feat(architect-to-leader): build sidebar nav (5 stages + reference)"
```

---

## Task 3: Front matter — Welcome + The Map

**Files:** Modify `gen-ai-architect-to-leader-guide.html`.

- [ ] **Step 1: Rewrite the `welcome` section**

Retitle to "Welcome to Gen AI: Architect to Leader". Content (cards):
- *Who this is for* — tech leads/EMs, platform/DevEx engineers, staff+/architects; assumes Part 1-2 fluency.
- *The gap this fills* — Parts 1-2 make an individual effective; this makes an org effective, safely and at scale.
- *How to read it* — training (top-to-bottom) vs reference (scorecard/checklists/metrics).
- *The three lenses* — People & Process, Platform & Infrastructure, Governance & Risk (with the three lens-tag pills shown).
- Series cross-link: `.info.purple` linking back to Part 1 and Part 2 (`href` to both files).

- [ ] **Step 2: Build the `map` section — the maturity model**

A `<section id="map" class="section">`: a `.pkt`-style 5-stage diagram (Ad-hoc → Pilot → Team → Org → Enterprise) and a `.tbl` with columns Stage / Name / Shape / Primary risk (content from spec §6).

- [ ] **Step 3: Add the lenses + Meridian intro to `map`**

- A short cards row defining the three lenses.
- A `.case` box introducing Meridian (~400-eng B2B SaaS) and the cast: skeptical staff engineer, ROI-focused VP, the quiet power user who becomes pilot lead, the platform team, the security lead. Note it's illustrative.

- [ ] **Step 4: Render & verify**

Run the per-step check (Conventions) with `<ids-built-this-task>` = `welcome map`. Expected: balanced divs, `ok welcome` and `ok map`. Browser: both sections render, stage diagram + table legible in dark/light.

- [ ] **Step 5: Commit**

```bash
git add gen-ai-architect-to-leader-guide.html
git commit -m "docs(architect-to-leader): front matter — Welcome + The Map"
```

---

## Task 4: Chapter 1 — Why Individual Mastery Doesn't Scale

**Lens:** People & Process. **Files:** Modify the HTML.

- [ ] **Step 1: Add `ch1-scale` section scaffold** — `<h1>1. Why Individual Mastery Doesn't Scale</h1>`, lens-tag `people`, "What you'll learn" preamble, `<h2>` placeholders for 1.1–1.4.
- [ ] **Step 2: Fill 1.1–1.2** — *1.1 The adoption cliff* (why "hand everyone a license" fails; readiness ≠ adoption). *1.2 Shadow usage & the consistency tax* (everyone's own prompts/configs; duplicated effort; invisible risk).
- [ ] **Step 3: Fill 1.3–1.4** — *1.3 The missing learning loop* (wins and failures don't propagate across people). *1.4 What "good at org scale" looks like* (preview the maturity model + three lenses).
- [ ] **Step 4: Meridian scene** — scattered ad-hoc usage; a skeptical staff engineer; a quiet power user shipping fast. (Stage 0.)
- [ ] **Step 5: See also** — `map`, `ch3-pilot`, `ch7-metrics`.
- [ ] **Step 6: Render & verify** (integrity check + browser).
- [ ] **Step 7: Commit** — `docs(architect-to-leader): write Ch 1 — Why Individual Mastery Doesn't Scale`.

---

## Task 5: Chapter 2 — The Business Case

**Lens:** People & Process · Governance & Risk. **Files:** Modify the HTML.

- [ ] **Step 1: Add `ch2-case` scaffold** (h1, lens-tags people+governance, preamble, h2 for 2.1–2.5).
- [ ] **Step 2: Fill 2.1–2.2** — *2.1 What leaders actually ask* ("what does this buy us, what's the risk, what does it cost"). *2.2 ROI without snake oil* (where time-savings are real vs illusory; value vs activity).
- [ ] **Step 3: Fill 2.3–2.4** — *2.3 Cost realities* (token spend, seats, hidden enablement/review costs; a worked illustrative cost table with a verify note). *2.4 Build-vs-buy & vendor posture* (single-tool vs multi-tool; lock-in; light treatment).
- [ ] **Step 4: Fill 2.5** — *2.5 Framing the ask* (a one-page pilot proposal template as a `.tbl` or step block).
- [ ] **Step 5: Meridian scene** — the VP wants numbers; the proposal one-pager is drafted.
- [ ] **Step 6: See also** — `ch3-pilot`, `ch7-metrics`, `ch11-cost`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 2 — The Business Case`.

---

## Task 6: Chapter 3 — Running a Credible Pilot

**Lens:** People & Process.

- [ ] **Step 1: Add `ch3-pilot` scaffold** (h1, lens-tag people, preamble, h2 for 3.1–3.5).
- [ ] **Step 2: Fill 3.1–3.2** — *3.1 Choosing the pilot team* (selection criteria; who NOT to pick; representativeness). *3.2 Defining success up front* (hypotheses, exit criteria, baseline capture).
- [ ] **Step 3: Fill 3.3–3.4** — *3.3 Time-boxing & guardrails* (duration, scope, safety rails for the pilot). *3.4 Avoiding the demo trap* (representative work, not cherry-picked wins; honest negatives).
- [ ] **Step 4: Fill 3.5** — *3.5 Capturing learnings* (what to record to carry into Stage 2).
- [ ] **Step 5: Meridian scene** — the pilot team forms around the power user; success criteria set.
- [ ] **Step 6: See also** — `ch1-scale`, `ch4-standards`, `ch7-metrics`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 3 — Running a Credible Pilot`.

---

## Task 7: Chapter 4 — Workflow Standards

**Lens:** People & Process · Platform & Infrastructure.

- [ ] **Step 1: Add `ch4-standards` scaffold** (h1, lens-tags people+platform, preamble, h2 for 4.1–4.4).
- [ ] **Step 2: Fill 4.1–4.2** — *4.1 From personal habits to shared conventions*. *4.2 Standardizing the loop* (spec / plan / TDD / review with agents, tool-agnostic; reference Part 2 Ch 6–11 for the individual mechanics).
- [ ] **Step 3: Fill 4.3–4.4** — *4.3 Where to write standards down* (living docs vs config; ownership). *4.4 Examples across tools* (the same convention expressed in Claude Code + a `.tool-alt` for Cursor/Kilo/Copilot).
- [ ] **Step 4: Meridian scene** — the team writes its first shared playbook.
- [ ] **Step 5: See also** — `ch8-config`, `ch5-enablement`, and a cross-guide note pointing to Part 2 Ch 6–11.
- [ ] **Step 6: Render & verify; Commit** — `docs(architect-to-leader): write Ch 4 — Workflow Standards`.

---

## Task 8: Chapter 5 — Enablement & Onboarding

**Lens:** People & Process.

- [ ] **Step 1: Add `ch5-enablement` scaffold** (h1, lens-tag people, preamble, h2 for 5.1–5.5).
- [ ] **Step 2: Fill 5.1–5.2** — *5.1 Leveling a team* (the skill-distribution problem). *5.2 Internal champions & train-the-trainer*.
- [ ] **Step 3: Fill 5.3–5.4** — *5.3 Pairing & review as teaching*. *5.4 Onboarding new hires* into the agentic workflow from day one.
- [ ] **Step 4: Fill 5.5** — *5.5 Anti-patterns* (over-reliance, review rubber-stamping, skill atrophy).
- [ ] **Step 5: Meridian scene** — champions emerge; an onboarding checklist is born.
- [ ] **Step 6: See also** — `ch4-standards`, `ch6-change`, `ch15-autonomy`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 5 — Enablement & Onboarding`.

---

## Task 9: Chapter 6 — Change Management

**Lens:** People & Process.

- [ ] **Step 1: Add `ch6-change` scaffold** (h1, lens-tag people, preamble, h2 for 6.1–6.4).
- [ ] **Step 2: Fill 6.1–6.2** — *6.1 The trust curve* (skepticism → over-trust → calibrated trust). *6.2 The "it shipped a bug" moment* (recovering trust after a public failure).
- [ ] **Step 3: Fill 6.3–6.4** — *6.3 Culture & incentives* (reward good practice, not raw output; avoid output theater). *6.4 Handling the loudest skeptic and the over-eager adopter*.
- [ ] **Step 4: Meridian scene** — an agent-shipped bug reaches prod; how the team responds (ties to Ch 14).
- [ ] **Step 5: See also** — `ch5-enablement`, `ch7-metrics`, `ch14-incident`.
- [ ] **Step 6: Render & verify; Commit** — `docs(architect-to-leader): write Ch 6 — Change Management`.

---

## Task 10: Chapter 7 — Measuring What Matters

**Lens:** People & Process · Governance & Risk.

- [ ] **Step 1: Add `ch7-metrics` scaffold** (h1, lens-tags people+governance, preamble, h2 for 7.1–7.5).
- [ ] **Step 2: Fill 7.1–7.2** — *7.1 Leading vs lagging indicators*. *7.2 Metrics that survive contact* (cycle time, review load, defect/escape rate, adoption depth) — `.tbl`.
- [ ] **Step 3: Fill 7.3–7.4** — *7.3 Vanity metrics & Goodhart's law* (lines-of-code, raw acceptance %; gaming). *7.4 Instrumenting honestly* (quant + qual; surveys + telemetry).
- [ ] **Step 4: Fill 7.5** — *7.5 Reporting up without overclaiming* (honest narrative to leadership).
- [ ] **Step 5: Meridian scene** — first metrics dashboard; a vanity metric gets retired.
- [ ] **Step 6: See also** — `ch2-case`, `ch6-change`, `ref-metrics`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 7 — Measuring What Matters`.

---

## Task 11: Chapter 8 — Golden Configuration as Code

**Lens:** Platform & Infrastructure.

- [ ] **Step 1: Add `ch8-config` scaffold** (h1, lens-tag platform, preamble, h2 for 8.1–8.5).
- [ ] **Step 2: Fill 8.1–8.2** — *8.1 Config sprawl* (the Stage 3 pain). *8.2 Golden config as a versioned, reviewed artifact* (treat agent rules like code).
- [ ] **Step 3: Fill 8.3–8.4** — *8.3 Tool-agnostic patterns* (a `.tbl`/`.cmp` mapping `CLAUDE.md` / `.cursorrules` / `AGENTS.md` / `.kilocode`; what's shared vs local). *8.4 Distribution & layering* (org → team → repo overrides; sync mechanics) with a `.tool-alt`.
- [ ] **Step 4: Fill 8.5** — *8.5 Keeping it from rotting* (ownership, change process, deprecation).
- [ ] **Step 5: Meridian scene** — the platform team replaces copy-paste configs with a golden baseline.
- [ ] **Step 6: See also** — `ch9-registry`, `ch10-cicd`, plus a cross-guide note to Part 2 Ch 2 (context) / Ch 13 (Claude Code config).
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 8 — Golden Configuration as Code`.

---

## Task 12: Chapter 9 — The Internal Capability Registry

**Lens:** Platform & Infrastructure.

- [ ] **Step 1: Add `ch9-registry` scaffold** (h1, lens-tag platform, preamble, h2 for 9.1–9.5).
- [ ] **Step 2: Fill 9.1–9.2** — *9.1 Skills & MCP servers as shared org capabilities*. *9.2 Building an internal registry* (discovery, versioning, trust/provenance).
- [ ] **Step 3: Fill 9.3–9.4** — *9.3 Central vs per-team* (what belongs where). *9.4 Distribution mechanics* (marketplaces, package registries, vendoring) with a `.tool-alt`.
- [ ] **Step 4: Fill 9.5** — *9.5 Maintaining quality* (review, ownership, deprecation of shared skills/servers).
- [ ] **Step 5: Meridian scene** — the shared skills + MCP registry launches; the first reused capability.
- [ ] **Step 6: See also** — `ch8-config`, `ch12-security`, plus a cross-guide note to Part 2 Ch 15 (skills) / Ch 16 (MCP).
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 9 — The Internal Capability Registry`.

---

## Task 13: Chapter 10 — Agents in CI/CD

**Lens:** Platform & Infrastructure · Governance & Risk.

- [ ] **Step 1: Add `ch10-cicd` scaffold** (h1, lens-tags platform+governance, preamble, h2 for 10.1–10.5).
- [ ] **Step 2: Fill 10.1–10.2** — *10.1 Deterministic pipeline vs agentic step* (where each belongs). *10.2 Automated review bots & agent-in-the-loop PR flows*.
- [ ] **Step 3: Fill 10.3–10.4** — *10.3 Merge gates* (what an agent may vs may not approve). *10.4 Headless/non-interactive runs in CI* (auth, budgets, timeouts) with a `.tool-alt` (e.g. Claude Code headless + GitHub Actions).
- [ ] **Step 4: Fill 10.5** — *10.5 Failure handling in pipelines* (flaky agents, retries, fallbacks).
- [ ] **Step 5: Meridian scene** — an agent review bot runs on every PR; a gate is tuned after a false-approve.
- [ ] **Step 6: See also** — `ch11-cost`, `ch13-gates`, `ch14-incident`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 10 — Agents in CI/CD`.

---

## Task 14: Chapter 11 — Sandboxing, Isolation & Cost Control at Scale

**Lens:** Platform & Infrastructure · Governance & Risk.

- [ ] **Step 1: Add `ch11-cost` scaffold** (h1, lens-tags platform+governance, preamble, h2 for 11.1–11.5).
- [ ] **Step 2: Fill 11.1–11.2** — *11.1 Isolation models at scale* (container / VM / devcontainer trade-offs). *11.2 Budgets, quotas & per-team accounting*.
- [ ] **Step 3: Fill 11.3–11.4** — *11.3 Model routing for cost* (right model per task; reference Part 2 Ch 4). *11.4 Runaway protection & circuit breakers*.
- [ ] **Step 4: Fill 11.5** — *11.5 The FinOps view* (forecasting, chargeback, showback).
- [ ] **Step 5: Meridian scene** — the first cost spike; quotas + routing + a circuit breaker are added.
- [ ] **Step 6: See also** — `ch2-case`, `ch10-cicd`, plus a cross-guide note to Part 2 Ch 4 (cost engineering).
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 11 — Sandboxing, Isolation & Cost Control`.

---

## Task 15: Chapter 12 — Security at Org Scale

**Lens:** Governance & Risk.

- [ ] **Step 1: Add `ch12-security` scaffold** (h1, lens-tag governance, preamble, h2 for 12.1–12.5).
- [ ] **Step 2: Fill 12.1–12.2** — *12.1 The expanded threat surface when agents act*. *12.2 Secrets & data egress* (what leaves the building; redaction; private deployments) — use `.info.red`.
- [ ] **Step 3: Fill 12.3–12.4** — *12.3 Prompt injection at org scale* (untrusted content & tool output; indirect injection) with `.info.red`; reference Part 2 Ch 3.4 / 16.11. *12.4 Supply-chain & dependency risk* (agent-added deps, third-party MCP servers).
- [ ] **Step 4: Fill 12.5** — *12.5 IP & license note* (generated-code provenance; light).
- [ ] **Step 5: Meridian scene** — the security review before org rollout; findings + mitigations.
- [ ] **Step 6: See also** — `ch9-registry`, `ch13-gates`, `ch14-incident`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 12 — Security at Org Scale`.

---

## Task 16: Chapter 13 — Review Gates, Policy & Compliance

**Lens:** Governance & Risk.

- [ ] **Step 1: Add `ch13-gates` scaffold** (h1, lens-tag governance, preamble, h2 for 13.1–13.5).
- [ ] **Step 2: Fill 13.1–13.2** — *13.1 The human-in-the-loop matrix* (what must a person approve; a `.tbl` by blast radius). *13.2 Audit trails* (logging agent actions/decisions).
- [ ] **Step 3: Fill 13.3–13.4** — *13.3 Policy as code vs policy as docs*. *13.4 Regulatory fit* (SOC2 / ISO / regulated industries; light).
- [ ] **Step 4: Fill 13.5** — *13.5 Gate friction vs throughput* (right-sizing approval).
- [ ] **Step 5: Meridian scene** — the review-gate policy is adopted org-wide.
- [ ] **Step 6: See also** — `ch10-cicd`, `ch12-security`, `ch15-autonomy`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 13 — Review Gates, Policy & Compliance`.

---

## Task 17: Chapter 14 — Failure Modes & Incident Response

**Lens:** Governance & Risk.

- [ ] **Step 1: Add `ch14-incident` scaffold** (h1, lens-tag governance, preamble, h2 for 14.1–14.5).
- [ ] **Step 2: Fill 14.1–14.2** — *14.1 Taxonomy of agent failures at org scale*. *14.2 When an agent does damage* (containment & rollback) — `.info.red`.
- [ ] **Step 3: Fill 14.3–14.4** — *14.3 The blameless postmortem for agent incidents*. *14.4 Feeding incidents back* into config/guardrails (close the loop).
- [ ] **Step 4: Fill 14.5** — *14.5 The on-call/runbook addendum* for agentic systems.
- [ ] **Step 5: Meridian scene** — an agent-caused incident, the postmortem, the guardrail that resulted (callback to Ch 6).
- [ ] **Step 6: See also** — `ch6-change`, `ch12-security`, `ch13-gates`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 14 — Failure Modes & Incident Response`.

---

## Task 18: Chapter 15 — Standardization vs Autonomy

**Lens:** Governance & Risk · People & Process.

- [ ] **Step 1: Add `ch15-autonomy` scaffold** (h1, lens-tags governance+people, preamble, h2 for 15.1–15.5).
- [ ] **Step 2: Fill 15.1–15.2** — *15.1 The central tension* (consistency vs velocity). *15.2 The paved-road / golden-path model*.
- [ ] **Step 3: Fill 15.3–15.4** — *15.3 Mandate vs recommend vs leave free* (a `.tbl`). *15.4 Governing without strangling* (escape hatches, feedback loops).
- [ ] **Step 4: Fill 15.5** — *15.5 Evolving standards as tools change* (avoiding fossilized rules).
- [ ] **Step 5: Meridian scene** — Meridian settles its paved-road model; teams keep an escape hatch.
- [ ] **Step 6: See also** — `ch4-standards`, `ch5-enablement`, `ch13-gates`.
- [ ] **Step 7: Render & verify; Commit** — `docs(architect-to-leader): write Ch 15 — Standardization vs Autonomy`.

---

## Task 19: Chapter 16 — Meridian's 12-Month Rollout (worked playbook)

**Lens:** Putting It Together (all three). **This is the capstone — it must reference earlier chapters explicitly by number.**

- [ ] **Step 1: Add `ch16-rollout` scaffold** (h1, "Putting It Together" framing, preamble, h2 for 16.1–16.7).
- [ ] **Step 2: Fill 16.1–16.2** — *16.1 The starting state (Stage 0)* (recap Ch 1). *16.2 Months 1–2: the pilot (Stage 1)* (decisions from Ch 2–3; outcomes).
- [ ] **Step 3: Fill 16.3–16.4** — *16.3 Months 3–5: team standardization (Stage 2)* (Ch 4–7 applied). *16.4 Months 6–9: org & platform (Stage 3)* (Ch 8–11 applied).
- [ ] **Step 4: Fill 16.5–16.6** — *16.5 Months 10–12: enterprise governance (Stage 4)* (Ch 12–15 applied). *16.6 What they'd do differently* (honest retrospective).
- [ ] **Step 5: Fill 16.7** — *16.7 Your turn* (how to adapt the playbook; pointer to the scorecard + checklists).
- [ ] **Step 6: Add a timeline visual** — a `.pkt` or step-block month-by-month strip mapping months → stages.
- [ ] **Step 7: See also** — `map`, `ref-scorecard`, `ref-checklists`.
- [ ] **Step 8: Render & verify; Commit** — `docs(architect-to-leader): write Ch 16 — Meridian's 12-Month Rollout`.

---

## Task 20: Reference — Maturity Scorecard + Adoption Checklists

**Files:** Modify the HTML.

- [ ] **Step 1: Build `ref-scorecard`** — a `<section id="ref-scorecard" class="section">` with a `.tbl` scoring each of the three lenses 0–4 (rows = lens, columns = stage descriptors), plus a short "how to score yourself" intro and a "where to jump next" mapping from score → Part.
- [ ] **Step 2: Build `ref-checklists`** — a `<section id="ref-checklists" class="section">` with one checklist block per stage transition (0→1 … 3→4); each is a point-wise `<ul>` of concrete "have you done X" items drawn from the matching chapters.
- [ ] **Step 3: Render & verify** (per-step check with `<ids-built-this-task>` = `ref-scorecard ref-checklists`; + browser).
- [ ] **Step 4: Commit** — `docs(architect-to-leader): reference — maturity scorecard + checklists`.

---

## Task 21: Reference — Metrics Catalog + Glossary + Further Reading

**Files:** Modify the HTML.

- [ ] **Step 1: Build `ref-metrics`** — a `.tbl` catalog: metric / what it measures / how to instrument / failure mode (gaming). Derived from Ch 7.
- [ ] **Step 2: Build `ref-glossary`** — a `<dl>` of org-adoption terms (maturity stage, paved road, golden config, capability registry, blast radius, human-in-the-loop, showback/chargeback, escape hatch, …), each with a chapter cross-link.
- [ ] **Step 3: Build `ref-reading`** — a labeled, hedged links list (DORA/engineering-effectiveness research, platform-engineering refs, AI-governance frameworks) with "verify current" framing.
- [ ] **Step 4: Render & verify** (per-step check with `<ids-built-this-task>` = `ref-metrics ref-glossary ref-reading`; + browser); **Commit** — `docs(architect-to-leader): reference — metrics catalog, glossary, further reading`.

---

## Task 22: Cross-link verification pass

**Files:** Modify the HTML (fix-ups only).

- [ ] **Step 1: Verify every `go('id')` resolves and every section is reachable**

```bash
cd /Users/divakaran/arrcus_workspace/guides
f=gen-ai-architect-to-leader-guide.html
grep -oE "go\('[a-z0-9-]+'\)" "$f"|sed "s/go('//;s/')//"|sort -u > /tmp/n.txt
grep -oE 'id="[a-z0-9-]+" class="section' "$f"|sed 's/id="//;s/" class.*//'|sort -u > /tmp/s.txt
echo "nav targets with no section:"; comm -23 /tmp/n.txt /tmp/s.txt
echo "sections never linked:"; comm -13 /tmp/n.txt /tmp/s.txt
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
```
Expected: both `comm` outputs empty; divs balanced. Fix any mismatch (typo'd id, missing section, stray see-also target).

- [ ] **Step 2: Verify all 23 sections present** — `grep -c 'class="section' gen-ai-architect-to-leader-guide.html` should be 23 (welcome, map, 16 chapters, 5 reference). Fix if short.
- [ ] **Step 3: Commit** (only if fixes were made) — `fix(architect-to-leader): cross-link + section integrity`.

---

## Task 23: Wire Part 3 into the series (Parts 1-2 + README)

**Files:** Modify `gen-ai-zero-to-hero-guide.html`, `gen-ai-hero-to-architect-guide.html`, `README.md`.

- [ ] **Step 1: Add a forward link to Part 3 in Zero-to-Hero** — in its "Where to Next" / welcome series box, add a third bullet/line linking `gen-ai-architect-to-leader-guide.html` as Part 3.
- [ ] **Step 2: Add a forward link to Part 3 in Hero-to-Architect** — in its welcome series note and the reference "Further Reading" companion card, add Part 3 with an `href`.
- [ ] **Step 3: Add the README row** — under the "Gen AI" table, add a row for `gen-ai-architect-to-leader-guide.html` with a one-line description.
- [ ] **Step 4: Verify links** — `grep -c 'href="gen-ai-architect-to-leader-guide.html"'` ≥ 1 in each of the two guides and the README.
- [ ] **Step 5: Commit** — `docs(gen-ai): wire Part 3 (Architect to Leader) into the series + README`.

---

## Task 24: Visual review pass

**Files:** none (review only; fix-ups if needed).

- [ ] **Step 1: Open the guide in a browser.** Walk every nav item top to bottom.
- [ ] **Step 2: Check** — dark/light toggle on a few sections; lens-tag pills render in their colors; `.case` boxes use the teal accent; tables/diagrams don't overflow; code blocks wrap; no console errors.
- [ ] **Step 3: Spot-check tool-agnostic rule** — every concrete config/command/CI example shows at least two tools (`grep -c 'tool-alt'` should be healthy; eyeball a few).
- [ ] **Step 4:** Fix any layout issues; commit `fix(architect-to-leader): visual polish` if changes made.

---

## Task 25: Full-guide super-review (R1)

**Files:** the HTML (fix-ups), plus a review doc.

- [ ] **Step 1 (manual/assisted — invoke the skill interactively, not a shell command): Run the `super-review` skill** on `gen-ai-architect-to-leader-guide.html` with criteria: technical accuracy, internal consistency (Meridian timeline numbers, stage names, lens tags, cross-links), tool-agnostic adherence, alignment with Parts 1-2 conventions and the model-version convention.
- [ ] **Step 2: Apply confirmed fixes** (HIGH → MEDIUM → LOW), re-verifying integrity after.
- [ ] **Step 3: Final commit** — `docs(architect-to-leader): R1 review fixes`. (Review doc `REVIEW-*.md` is gitignored.)

---

## Self-Review Notes (writer to writer)

- **Spec coverage:** every spec §9 chapter maps to a task (Ch 1–16 → Tasks 4–19; front matter → Task 3; reference §9 → Tasks 20–21). Spec §5 framing rules are encoded in the Conventions block (tool-agnostic, lens tags, Meridian scenes, hedged volatile facts, see-also grids). Spec §8 visual style → Task 1 (chrome + accent + components). Spec §6 maturity model → Task 3 (Map) + Task 20 (scorecard). Spec §7 Meridian → every chapter's scene step + Task 19 capstone. Cross-linking both ways (§4/§8) → Task 23.
- **Non-goals respected:** no "building AI products" content; build-vs-buy and IP/license stay light (Ch 2.4, Ch 12.5); no single-tool manual (tool-agnostic rule enforced in Conventions + Task 24 spot-check).
- **Id consistency:** section ids used in nav (Task 2), per-chapter scaffolds (Tasks 4–19), reference (Tasks 20–21), and see-also targets all use the same kebab ids (`ch1-scale` … `ch16-rollout`, `ref-scorecard` … `ref-reading`). Task 22 mechanically verifies they match.
- **Verification is content-appropriate:** no pytest; "tests" are the HTML integrity check (tag balance + nav/section parity) run every render step, the Task 22 link pass, the Task 24 visual pass, and the Task 25 super-review.
- **Commit cadence:** one commit per task (scaffolding = `feat`, content = `docs`, fix-ups = `fix`), matching Part 2's history.
