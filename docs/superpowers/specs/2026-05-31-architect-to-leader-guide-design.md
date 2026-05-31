# Design Spec — Gen AI for Developers: Architect to Leader

**Status:** Approved skeleton, pending user review of spec before plan
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-05-31
**Part of series:** Part 3 of "Gen AI for Developers"
**Builds on:** `gen-ai-zero-to-hero-guide.html` (Part 1), `gen-ai-hero-to-architect-guide.html` (Part 2)
**Output artifact:** `gen-ai-architect-to-leader-guide.html` (single-page HTML, same chrome family as Parts 1-2)

---

## 1. Purpose

Parts 1-2 make an *individual* developer effective with agentic coding. This guide answers the next question: **how does an organization get there** — moving from scattered, ad-hoc individual use to a safe, consistent, measurable, org-wide practice — without chaos, security incidents, or a pile of inconsistent configs.

It is a **comprehensive adoption playbook** spanning three lenses — **People & Process**, **Platform & Infrastructure**, and **Governance & Risk** — structured as a staged maturity journey and narrated through a recurring company case study.

Serves two reader modes:
- **Training** — linear top-to-bottom read for a leader planning or running a rollout.
- **Reference** — dip-in lookup (maturity scorecard, per-stage checklists, metrics catalog) once the rollout is underway.

## 2. Audience

Comprehensive — the people who drive and sustain org adoption, addressed together:
- **Tech leads / engineering managers** — people + process, change management, measurement.
- **Platform / DevEx engineers** — shared infrastructure, golden config, registries, CI integration, cost controls.
- **Staff+ engineers / architects** — standards, security, governance, risk across teams.

Assumes the reader (or their team) has Part 1-2 level fluency in agentic coding. This guide does **not** re-teach how to use an agent; it teaches how to make a whole org good at it.

## 3. Non-goals

- Not a re-teach of individual agentic-coding skills (that's Parts 1-2).
- Not about building AI-powered *products* that call LLM APIs (RAG systems, agents-as-products, evals) — that is a different, candidate future guide ("Architect to Builder").
- No vendor procurement/contract-negotiation deep dive — light treatment folded into Ch 2.
- No legal/IP/open-source-license deep dive — light treatment folded into Ch 12-13.
- Not a single-tool manual. Principles are tool-agnostic (see §5).

## 4. Positioning vs. Parts 1-2

| | Part 1 — Zero to Hero | Part 2 — Hero to Architect | Part 3 — Architect to Leader |
|---|---|---|---|
| Reader | New to AI | Individual picking up agentic coding | Leaders scaling it across an org |
| Altitude | Concepts | Individual practice | Organizational practice |
| Question | "What is this?" | "How do I work this way?" | "How does my org work this way — safely, at scale?" |
| Tooling | Claude Code intro | Claude/Kilo/Gemini deep dives | Tool-agnostic, principles-first |
| Spine | Topic progression | Worked example (`gitlog-report`) | Maturity model + case study (Meridian) |

## 5. Framing rules (apply throughout)

1. **Tool-agnostic, principles-first.** Every topic is framed as a principle that applies to any agentic coding tool. Concrete examples are given across **Claude Code, Cursor, Kilo Code, and Copilot** (e.g. golden config shown as `CLAUDE.md` / `.cursorrules` / `.kilocode` / `AGENTS.md`), never assuming one vendor.
2. **Three lenses, declared per chapter.** Every chapter is tagged with which lens(es) it serves — People & Process, Platform & Infrastructure, Governance & Risk — so a reader can navigate by concern as well as by stage.
3. **Stage-led, chronological.** Parts map to maturity stages and follow Meridian's journey start-to-finish; a lens appears within a stage when that stage demands it.
4. **The case study (Meridian) threads through every Part** as short scene-setters at the start of each Part and recurring callouts, then gets the full end-to-end treatment in Ch 16.
5. **Every chapter ends with a "see also" cross-link grid** to related chapters (and, where relevant, back to specific Part 1-2 chapters).
6. **Volatile facts are hedged.** Pricing, model names, and tool-specific commands carry "verify current" notes — consistent with Part 2's discipline. Model references follow the series convention (Claude Opus/Sonnet 4.7, GPT-5, Gemini 3) where named at all; the guide prefers capability language over version numbers given its tool-agnostic stance.

## 6. The maturity model (the backbone)

A 5-stage adoption journey. Each stage is the lens for a group of chapters and a row in the reference self-assessment scorecard.

| Stage | Name | Shape | Primary risk |
|---|---|---|---|
| 0 | Ad-hoc | Scattered individuals, no shared practice (where Parts 1-2 leave you) | Inconsistency, shadow usage, no learning loop |
| 1 | Pilot | One small team proves value with a time-boxed experiment | Demo theater; unrepresentative wins |
| 2 | Team | A team standardizes workflow, conventions, and enablement | Process ossification; uneven skill |
| 3 | Org | Multiple teams; a platform and shared capabilities emerge | Config sprawl, cost runaway, duplicated effort |
| 4 | Enterprise | Governance, compliance, and measurement at scale | Over-governance killing velocity; audit/security gaps |

## 7. The case study — Meridian

A fictional **~400-engineer B2B SaaS company**. The narrative follows its rollout stage by stage, giving every abstract topic a concrete anchor:

- **Stage 0→1:** A skeptical staff engineer; a VP who wants ROI numbers; a quietly-successful skunkworks user who becomes the pilot lead.
- **Stage 1→2:** The pilot team standardizes; the first "it shipped a bug" incident and how trust is rebuilt.
- **Stage 2→3:** A platform team forms; golden config and an internal registry replace copy-paste; the first cost spike.
- **Stage 3→4:** Security review; an agent-caused incident and postmortem; the board's "what did this buy us?" question.
- **Putting It Together (Ch 16):** Meridian's full 12-month timeline assembled as a replayable playbook, revisiting stages 0→4.

Meridian is illustrative, not a real company; numbers are plausible and labeled as illustrative.

## 8. Visual style

Exact match to the Parts 1-2 chrome:
- Sidebar navigation (left, fixed, grouped by Part/stage), dark/light toggle.
- JetBrains Mono code, serif headings, sans body; same color tokens and component vocabulary (info boxes incl. `.info.red` for risk callouts, comparison grids, step blocks, tables, packet-style diagrams adapted for org/architecture diagrams).
- A distinguishable accent so a reader can tell at a glance they're in Part 3 (specific accent chosen during build).
- Cross-links to Part 1 and Part 2 via the series convention (welcome note + reference card), and Parts 1-2 will gain a forward link to Part 3 on completion.

## 9. Structure — chapter outline

**Front Matter**
- **Welcome** — who this is for, how it builds on Parts 1-2, the promise.
- **The Map** — the 5-stage maturity model, the three lenses, and meet Meridian.

**Part 1 · Stage 0→1 — Ad-hoc to Pilot: Making the Case** *(lenses: People)*
1. **Why individual mastery doesn't scale** — the org adoption problem; shadow usage; the learning-loop gap.
2. **The business case** — ROI framing, cost realities, and the metrics leaders actually ask about; light build-vs-buy / vendor note.
3. **Running a credible pilot** — choosing the team, defining success, time-boxing, avoiding the demo trap.

**Part 2 · Stage 1→2 — Pilot to Team: People & Process** *(lenses: People)*
4. **Workflow standards** — shared, tool-agnostic conventions for spec / plan / TDD / review with agents.
5. **Enablement & onboarding** — leveling a team, internal champions, pairing, common anti-patterns.
6. **Change management** — skeptics, trust, the "it shipped a bug" moment, culture & incentives.
7. **Measuring what matters** — leading vs lagging indicators, instrumenting them, avoiding vanity metrics & gaming.

**Part 3 · Stage 2→3 — Team to Org: Platform & Infrastructure** *(lenses: Platform)*
8. **Golden configuration as code** — shared agent rules/config as versioned artifacts (`CLAUDE.md` / `.cursorrules` / `AGENTS.md` / …); distribution and versioning.
9. **The internal capability registry** — building, sharing, and distributing skills and MCP servers across teams.
10. **Agents in CI/CD** — automated review, agent-in-the-loop pipelines, merge gates, deterministic vs agentic steps.
11. **Sandboxing, isolation & cost control at scale** — budgets, quotas, model routing, runaway protection, per-team accounting.

**Part 4 · Stage 3→4 — Org to Enterprise: Governance, Security & Risk** *(lenses: Governance)*
12. **Security at org scale** — secrets, data egress, prompt injection, supply-chain & dependency risk; light IP/license note.
13. **Review gates, policy & compliance** — what a human must approve, audit trails, regulatory fit.
14. **Failure modes & incident response** — when an agent does damage; postmortems; the rollback playbook.
15. **Standardization vs. autonomy** — governing without strangling velocity; the paved-road / golden-path model.

**Part 5 · Putting It Together**
16. **Meridian's 12-month rollout** — the full journey end-to-end as a worked playbook, replaying the maturity model.

**Reference**
- **Maturity self-assessment scorecard** — rate your org 0-4 on each lens.
- **Per-stage adoption checklists.**
- **Metrics catalog** — definitions + how to instrument each.
- **Glossary** — org-adoption terms.
- **Further reading** — labeled, hedged links.

Target scale: 16 chapters across 5 parts + front matter + reference — comparable to Part 2.

## 10. Success criteria

- A leader can read it top-to-bottom and come away with a concrete, staged rollout plan plus the metrics, configs, and guardrails each stage needs.
- The maturity scorecard lets a reader locate their org and jump to the right Part.
- Tool-agnostic: no example assumes a single vendor; at least Claude Code + one other tool shown for each concrete artifact.
- Internally consistent with Parts 1-2 in voice, components, terminology, and the model-version convention; cross-linked both ways.
- Self-contained single HTML file, no build step, opens in any browser.

## 11. Build approach

Implementation via the `guide-builder` skill (single self-contained HTML, series template). Likely build order: chrome + nav + maturity-model/Map front matter → Meridian case-study spine → Parts 1-5 chapters → reference pages → cross-link Parts 1-2 forward to Part 3 → full-guide super-review (R1+). Detailed sequencing belongs in the implementation plan (writing-plans).
