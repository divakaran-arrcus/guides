# Design Spec — Gen AI for Developers: Hero to Architect

**Status:** Approved skeleton, pending user review of spec before plan
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-05-25
**Companion to:** `gen-ai-zero-to-hero-guide.html` (v1)
**Output artifact:** `hero-to-architect-guide.html` (single-page HTML, same chrome family as v1)

---

## 1. Purpose

A comprehensive, deep-dive companion to the v1 "Zero to Hero" guide for developers picking up agentic coding tools (primarily Claude Code, Kilo Code, and Gemini CLI) and the methodology stack around them (OpenSpec, SpecKit, Superpowers, MCP). v1 is breadth-first ("what is this"); this guide is depth-first ("how do I work this way").

Serves two reader modes:
- **Training** — linear top-to-bottom read for a developer ramping up.
- **Reference** — dip-in lookup once the reader is working day-to-day.

## 2. Audience

A developer who is just picking up an AI coding agent (Claude Code, Kilo Code, or Gemini CLI) and wants to wield it well, including the spec-driven + skills/superpowers methodology layered on top. Assumes the reader has read v1 (or equivalent breadth knowledge of LLMs, prompting, RAG basics).

## 3. Non-goals

- No transformer math, attention diagrams, or model-internals deep dives (lives in v1, lightly).
- No Cursor/Windsurf/Aider/Antigravity deep dives — comparison cards only.
- No deep dive on Codex CLI — flagged as candidate for a future companion.
- This guide is about working *with* agents, not building agentic products that call the Claude/OpenAI/Gemini APIs directly (that's a different guide).

## 4. Positioning vs. v1

| | v1 (Zero to Hero) | v2 (Hero to Architect) |
|---|---|---|
| Reader | New to AI | Picking up agentic coding tools |
| Treatment | Breadth — what is this | Depth — how do I work this way |
| Section length | Short, many | Deep, fewer |
| Tools | Claude Code intro | Kilo Code + Claude Code + Gemini CLI deep dives + methodology layer |
| Style | Foundational | Practitioner + reference |

## 5. Framing rules (apply throughout)

1. **Concept-first, then tool-specific.** Every concept gets a tool-agnostic mental-model treatment first, then implementation in the tools that have it.
2. **Kilo Code primary in IDE contexts; Claude Code primary in terminal contexts; Gemini CLI as the free-tier-friendly terminal alternative.** Avoids three-way fragmentation in every example.
3. **Antigravity stays as a callout** in Ch 5.9 and the decision matrix — no deep dive.
4. **Every chapter ends with a "see also" cross-link grid** to related chapters elsewhere in the guide.
5. **The CLI worked example (`gitlog-report`) threads through Ch 6–11** as 1-page mini-walkthroughs at the end of each methodology chapter, then gets the full end-to-end treatment in Ch 17.
6. **"Kilo Code primary, Claude/Gemini as alt" callouts** appear next to any tool-specific example.

## 6. Visual style

Exact match to the v1 guide chrome:
- Sidebar navigation (left, fixed, grouped by Part)
- Dark/light mode toggle
- JetBrains Mono code blocks, Source Serif 4 headings, DM Sans body
- Same color palette tokens (`--bg`, `--surface`, `--blue`, `--cyan`, etc.)
- Same component vocabulary: info boxes, comparison grids, packet-style diagrams adapted for AI-architecture diagrams, tables
- Hero treatment: same family but distinguishable accent (Part 2 differentiator TBD during build — likely a subtle hero color shift so the reader can tell at a glance which guide they're in)

## 7. Worked example

**Project:** `gitlog-report` — a small CLI that ingests `git log` from a repository and produces a per-author activity summary (commits/day, lines changed, top files touched).

**Why this project:**
- Bounded scope (one CLI, no infra).
- Clear acceptance criteria (testable behavior — given a log, produce a JSON/markdown summary).
- Exercises every methodology lever cleanly: spec, plan, TDD, debug, review, parallel work.
- Runs anywhere — readers can follow along on any machine.

**Threading through the guide:**
- Ch 6 — write the spec (OpenSpec + SpecKit shown side-by-side)
- Ch 7 — write the plan
- Ch 8 — write the failing tests
- Ch 9 — inject a bug, debug it
- Ch 10 — full review pass (`/security-review`, `/ultrareview`)
- Ch 11 — parallel feature+tests+docs polish via subagents
- Ch 17 — stitch the full pipeline end-to-end in Kilo Code (Pass 1) and Claude Code (Pass 2), then "highlights only" in Gemini CLI (Pass 3), with cost / time / ease compare

## 8. Structure (5 Parts, 17 chapters, 4 reference pages)

### Front matter
- Welcome — who this is for, prerequisites (v1), how to read as training vs reference
- The map — three layers and how to navigate

### Part 1 — Foundations of Agentic Coding *(the mental model)*

**Ch 1. The Agentic Coding Loop** *(11 subsections)*
The plan → act → observe loop, what differs from chat, control problems, plan vs auto mode, common failure modes, hands-on trace.

**Ch 2. Context as Working Memory** *(11 subsections)*
Token economy, prompt caching, compaction, when to clear, reading the budget, memory systems (CLAUDE.md, GEMINI.md, `.kilocoderules`), anti-patterns.

**Ch 3. Tools, Side Effects & the Agent's World** *(11 subsections)*
Tool primitives, blast radius, idempotency, permission models, allowlists, sandboxes, observability.

**Ch 4. Model Selection & Cost Engineering** *(featured, 14 subsections)*
Claude / GPT / Gemini / local model families, the three dials (capability/cost/latency), BYO-model in Kilo, thinking mode, caching, batching, decision tree, hands-on budget exercise.

**Ch 5. IDE vs Terminal: Choosing Your Surface** *(featured, 13 subsections)*
Kilo Code / Claude Code / Gemini CLI first-runs, IDE-vs-terminal strengths, expanded comparison cards for Cursor/Windsurf/Antigravity/Cline/Continue/JetBrains and Aider/Copilot-CLI/Codex CLI, decision matrix, hands-on same-task-two-tools exercise.

### Part 2 — The Methodology Layer *(how you actually work)*

**Ch 6. Spec-Driven Development: OpenSpec & SpecKit** *(featured, 11 subsections)*
Methodology concept-first, then both implementations: OpenSpec (spec → change → archive) and SpecKit (`/specify` → `/plan` → `/tasks` → `/implement`), side-by-side comparison, decision matrix, mini-walkthrough — **write the spec for gitlog-report**.

**Ch 7. Plan-Driven Execution** *(11 subsections)*
Plan vs spec, brainstorm→spec→plan→execute rhythm, plan files as artifacts, plan mode, replanning, when to skip, cross-session handoff, mini-walkthrough — **write the plan**.

**Ch 8. Test-Driven Development with Agents** *(11 subsections)*
TDD as control rod, red-green-refactor with agents, golden tests, snapshot dangers, anti-mocking discipline, eval loops, mini-walkthrough — **write the failing tests**.

**Ch 9. Debugging with Agents** *(11 subsections)*
Systematic debugging loop, bugs the agent fixes wrong, root cause vs patch, evidence feeding, when to step in, debugging-the-agent, mini-walkthrough — **inject a bug, debug it**.

**Ch 10. Safety, Review & Verification** *(featured, 11 subsections)*
Three review layers, `/security-review`, `/ultrareview`, verification-before-completion, hallucinations in code, the "looks right vs is right" gap, sign-off gates, mini-walkthrough — **full review pass**.

**Ch 11. Multi-Agent & Parallel Workflows** *(featured, 11 subsections)*
Subagents, worktrees, `/loop`, `/schedule`, agent-of-agents, Kilo Orchestrator mode, when parallel hurts, token math, mini-walkthrough — **feature + tests + docs in parallel**.

### Part 3 — Mastery: The Power Layer *(the tools that make it real)*

**Ch 12. Kilo Code Deep Dive** *(featured, 14 subsections)*
Architecture, the five modes (Architect/Code/Ask/Debug/Orchestrator), BYO-model per mode, `.kilocodemodes`, custom rules, settings, Memory/Context Bank, MCP, commit/PR features, cost dashboard, monorepo, power-user shortcuts, hands-on.

**Ch 13. Claude Code Deep Dive** *(featured, 14 subsections)*
Architecture, CLAUDE.md, slash commands, skills, plugins, hooks, sub-agents, plan mode, output styles, fast mode, ScheduleWakeup/`/loop`/`/schedule`, settings & permissions, auto-memory, hands-on.

**Ch 14. Gemini CLI Deep Dive** *(featured, 15 subsections)*
Architecture, the model story (Gemini 3 Pro/Flash + free-tier economics), authentication paths (personal Google / API key / Vertex AI), GEMINI.md, slash commands, skills + `activate_skill`, extensions, hooks, built-in tools, MCP integration, settings, memory tool, 1M-context implications, hands-on.

**Ch 15. The Superpowers Methodology — Claude Code Skills, Gemini CLI Skills & Kilo Code Modes** *(featured, 15 subsections)*
Methodology bundles as portable patterns, three implementations (Claude Code skills, Gemini CLI skills, Kilo Code modes/rules), the Superpowers Claude Code plugin (canonical implementation, `using-superpowers` auto-invocation), the skill format, skill discovery, rigid-vs-flexible, adapting to Kilo Code via custom modes, what you give up in Kilo Code, composition, testing, sharing, anti-patterns, hands-on — ship a methodology bundle for your team.

**Ch 16. MCP Servers — Using and Building** *(featured, 14 subsections)*
What MCP is, three primitives (tools/resources/prompts), stdio vs HTTP/SSE, tour of existing servers (Atlassian/Slack/Box/Figma/GitHub), adding MCP to Kilo Code and Claude Code, authoring (Python/TypeScript), server skeleton, designing tools agents use well, resources, prompts, auth & secrets, testing, publishing, hands-on — build a tiny local-notes MCP server.

### Part 4 — Putting It All Together

**Ch 17. Worked Example: Ship `gitlog-report` End-to-End** *(featured, 15 subsections)*
- Brief + acceptance criteria
- Why multiple passes
- **Pass 1 (Kilo Code)** — brainstorm + spec + plan → TDD → debug injected bug → review → ship *(5 subsections)*
- **Pass 2 (Claude Code)** — brainstorm + spec + plan → parallel subagent execution → verification/security/ultrareview → ship *(4 subsections)*
- **Pass 3 (Gemini CLI)** — highlights only, key differences vs Passes 1 & 2 *(1 subsection)*
- Side-by-side: cost, time, ease, free-tier economics
- Lessons learned
- See also

### Part 5 — Reference *(skim, dip, look up)*

**Decision Frameworks** *(matrices)*
- Which tool — Kilo Code vs Claude Code vs Gemini CLI vs Antigravity (with Cursor/Windsurf row)
- Which model for which task
- Which mode/skill for which task
- OpenSpec vs SpecKit
- When to plan vs skip
- When to parallelize
- When to engage thinking mode

**Cheat Sheets**
- Kilo Code commands, modes, shortcuts
- Claude Code slash commands, skills, hooks
- Gemini CLI slash commands, extensions, skills
- MCP — popular servers at a glance + install snippets
- OpenSpec command reference
- SpecKit command reference (`/specify`, `/plan`, `/tasks`, `/implement`)
- Superpowers — skill library at a glance

**Glossary A–Z**
Extends v1's glossary with: skill, plugin, hook, MCP, sub-agent, worktree, spec, plan file, mode (Kilo), CLAUDE.md, GEMINI.md, `.kilocoderules`, prompt cache, thinking mode, golden test, eval loop, Orchestrator mode, side-effect tool, blast radius, agent loop, headless, BYO model, `activate_skill`, extensions (Gemini), free-tier mechanics.

**Further Reading**
Canonical docs (Claude Code, Kilo Code, Gemini CLI, MCP, OpenSpec, SpecKit, Superpowers); curated reading list; cross-links back to v1 and the networking guides.

## 9. Subsection counts at a glance

| Part | Chapters | Subsection total |
|---|---|---|
| Part 1 — Foundations | 5 (Ch 1–5) | 11+11+11+14+13 = 60 |
| Part 2 — Methodology | 6 (Ch 6–11) | 11×6 = 66 |
| Part 3 — Mastery | 5 (Ch 12–16) | 14+14+15+15+14 = 72 |
| Part 4 — Worked Example | 1 (Ch 17) | 15 |
| Part 5 — Reference | 4 pages | ~30 entries |
| **Total chapters / subsections** | **17 chapters** | **~213 subsections + ~30 reference entries** |

## 10. Cross-cutting threads (woven through chapters)

1. **Worked-example mini-walkthroughs** at the end of Ch 6–11 (cumulative — spec, plan, tests, debug, review, parallel polish), full end-to-end in Ch 17.
2. **"See also" link grids** at the end of every chapter.
3. **"Kilo Code primary, Claude/Gemini as alt" callouts** wherever a tool-specific example appears.
4. **Card-format mini-tours** for adjacent tools (Cursor, Windsurf, Antigravity, etc.) so they're positioned without warranting full coverage.

## 11. Out-of-scope deferrals (candidates for v3)

- Codex CLI deep dive
- Antigravity deep dive (revisit when MCP support and ecosystem solidify)
- Building agentic products that call LLM APIs directly
- Fine-tuning, LoRA, distillation
- Inference serving (vLLM, quantization, GPUs)
- AI infra at scale / enterprise governance deep dive

## 12. Implementation outline (for the writing-plans skill to expand)

The implementation plan will need to address, in roughly this order:

1. **HTML scaffold** — copy v1's chrome (sidebar, dark/light toggle, fonts, color tokens, body grid). Adjust title + hero accent so the reader can tell at a glance which guide they're in.
2. **Sidebar grouping** — five Part labels, 17 chapter buttons, 4 reference page buttons.
3. **Section-by-section content writing** — Part 1 → Part 2 → Part 3 → Part 4 → Part 5, in order (so cross-references can be built progressively).
4. **Worked-example continuity** — write Ch 6's `gitlog-report` spec first, then ensure Ch 7–11 mini-walkthroughs and Ch 17 full pipeline stay consistent with that spec.
5. **Visual components** — adapt existing comparison grids, info boxes, tables; introduce one new component family for agent-loop / multi-agent diagrams.
6. **Reference pages** — populate the decision matrices and cheat sheets from the chapter content (don't write twice; cross-link).
7. **Glossary** — extend v1's glossary as a delta layer; don't repeat v1's entries.
8. **Review pass** — each Part gets a self-review for placeholders, contradictions, completeness before moving to the next Part.

## 13. Success criteria

- A developer who has read v1 can finish v2 and confidently pick up Kilo Code, Claude Code, or Gemini CLI and start shipping with a spec → plan → TDD → review workflow.
- The guide stands up as a reference: any reader can search the sidebar for "MCP" or "hooks" or "thinking mode" and find a coherent standalone explanation.
- The worked example reads as one continuous story when followed Ch 6 → 11 → 17, and also stands alone in Ch 17.
- The visual style sells the "Part 2 of a series" feeling — same family as v1, immediately recognizable.

## 14. Open items at spec-writing time

- The hero accent color shift for differentiation from v1 — decide during HTML scaffold.
- Whether reference pages live in the sidebar as separate entries or under a single "Reference" disclosure — decide during scaffold.
- The exact `gitlog-report` acceptance criteria — drafted in Ch 6 itself; this spec just commits to the *shape* of the tool.
- Antigravity card content depth in 5.9 — keep proportional to other cards; revisit if it ships MCP support before the guide does.
