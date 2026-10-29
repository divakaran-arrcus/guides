# Harness / Loop / Graph Engineering — Verified Sources (as of 2026-10-28)

The **only** citation list content writers may use for Part 4. Every entry below was fetched on 2026-10-28.
Format: **title** · URL · date · verified claim · tag (`solid` = cite as authority, `secondary` = color only).
IDs in brackets (e.g. `[S3]`) are what "Key facts" refer to.

## Solid

- **[S1] Harness engineering: leveraging Codex in an agent-first world** (Ryan Lopopolo, OpenAI) · https://openai.com/index/harness-engineering/ · 2026-02-11 · The team's job shifts from writing code to designing environments, specifying intent and building feedback loops; ~5-month experiment, ~1M lines of code, no hand-written code. · `solid`
  - Note: page returns HTTP 403 to automated fetch. Title/date corroborated by multiple outlets; figures verified via [S16]. Cite the post for the idea; cite [S16] for numbers.
- **[S2] The Anatomy of an Agent Harness** (Vivek Trivedy, LangChain) · https://www.langchain.com/blog/the-anatomy-of-an-agent-harness · 2026-03-10 · States "Agent = Model + Harness"; "A harness is every piece of code, configuration, and execution logic that isn't the model itself." Components: system prompts; tools/skills/MCP; bundled infra (filesystem, sandbox, browser); orchestration (subagents, handoffs, routing); hooks/middleware (compaction, continuation, lint checks). · `solid`
- **[S3] Harness Engineering: Anatomy, Architecture, and Evolution of Coding Agents — A Source-Code Study of Eleven Systems** (Barbaste et al.) · https://arxiv.org/abs/2609.00006 · 2026-07-15 (v1) · Source-code study of 11 production coding-agent harnesses (+1 meta-harness); catalogues 29 recurring design patterns; none use a general-purpose agent framework or vector-embedding retrieval; the harness has grown from tool to platform. · `solid`
- **[S4] Agentic Harness Engineering: Observability-Driven Automatic Evolution of Coding-Agent Harnesses** (Lin et al.) · https://arxiv.org/abs/2604.25850 · 2026-04-28 (v1; v4 2026-05-18) · Closed loop auto-evolves a harness; each edit paired with a prediction checked next round. 10 iterations: Terminal-Bench 2 pass@1 69.7% → 77.0% (vs. human-designed Codex-CLI harness 71.9%). Ablations credit tools, middleware and long-term memory more than the system prompt. · `solid`
- **[S5] JIT-Agent: Scaling Harness Intelligence via Just-in-Time Harness Evolution** (Zhang et al.) · https://arxiv.org/abs/2608.25593 · 2026-08-26 (v1; v2 2026-09-03) · A trained model generates task-specific harnesses (memory, planning, action protocol, tool orchestration) on the fly and repairs them from an archive of past configs; reports large gains for weaker base models. · `solid` (research direction — hedge benchmark numbers)
- **[S6] Loop Engineering: Building Blocks, Adoption, and Impact** (Lulla, Nersesyan, Mohsenimofidi, Treude, Baltes) · https://arxiv.org/abs/2608.21884 · 2026-08-22 (v1; v3 2026-10-22) · Gray-literature review: practitioners agree on loop components but there is **no systematic evidence yet** for claimed benefits. Mining 217 GitHub repos: loops usually commit startup config but rarely commit the recommended state files. · `solid`
- **[S7] From Static Templates to Dynamic Runtime Graphs: A Survey of Workflow Optimization for LLM Agents** (Yue et al.) · https://arxiv.org/abs/2603.22386 · 2026-03-23 · Treats LLM workflows as "agentic computation graphs"; classifies methods by whether structure is fixed before deployment or chosen/revised per run; separates templates, per-run graphs and execution traces. · `solid`
- **[S8] Harness as an Asset: Enforcing Determinism via the Convergent AI Agent Framework (CAAF)** (Tianbao Zhang) · https://arxiv.org/abs/2604.17025 · 2026-04-18 (v1; v3 2026-05-04) · Task decomposition + machine-readable domain-rule registry enforced by a fixed assertion interface + state-locked feedback make agent workflows more deterministic; ablations suggest all three are needed. · `solid` (single-author; use as an example, not consensus)
- **[S9] From Prompts to Contracts: Harness Engineering for Auditable Enterprise LLM Agents** (Ahn, Kim) · https://arxiv.org/abs/2607.08028 · 2026-07-09 · Moving validation rules/schemas out of prompts into code made agents auditable; only code-owned enforcement blocked violations without utility loss (120/120); a bolt-on guardrail over-refused (88/120). · `solid`
- **[S10] Distributing Security Controls Through Harness Engineering** (W. R. Gore) · https://arxiv.org/abs/2607.25890 · 2026-07-28 · OS sandboxing, skill scanning and tool restriction packaged into a custom harness scored on a 23-test suite based on OWASP Top 10 for Agentic Applications; notes model non-determinism yields inconsistent security outcomes. · `solid`
- **[S11] Harness Engineering for Self-Improvement** (Lilian Weng, Lil'Log) · https://lilianweng.github.io/posts/2026-07-04-harness/ · 2026-07-04 · Once a harness is code, an LLM can search over it (cites Meta-Harness, ADAS, AFlow); evaluators are the central bottleneck; the evaluator and permission controls should sit **outside** the loop that evolves the harness to limit reward hacking; a capable base model is still required (STOP: helped GPT-4, degraded weaker models). · `solid`
  - Paper count: secondaries say "35 papers"; the post itself does not state a count (39 reference entries). Don't quote a number.
- **[S12] What is loop engineering?** (Ivan Belcic, Cole Stryker, IBM Think) · https://www.ibm.com/think/topics/loop-engineering · 2026-07-17 · "An emerging agentic engineering practice": designing loops that repeatedly steer agents toward goals with little human input. Stages: goal → action → observation → adjustment. Components: automations/scheduling, hooks, context engineering, tool access, worktrees, skills, subagents, spine. · `solid`
- **[S13] 3 Years of Graph Engineering with LangGraph** (Sydney Runkle, Harrison Chase, LangChain) · https://www.langchain.com/blog/3-years-of-graph-engineering-with-langgraph · 2026-07-22 · Graph engineering is a new name for an established practice. Nodes = code, single LLM call, tool call, or full agent; edges fixed or conditional; production graphs need cycles (retries, validation, human approval) so are usually not DAGs; loops are a simple form of graph. What changed: a node can now be a whole agent run. Some open-ended tasks suit a harness better than a predefined graph. · `solid`
- **[S14] Beyond the Prompt: Why Harness Engineering Is a Lever for AI Agents** (Alexis Ageorges, Palo IT) · https://www.palo-it.com/en/blog/beyond-the-prompt-why-harness-engineering-is-a-lever-for-ai-agents · undated (refs to Jul 2026) · Nested layers: prompt ⊂ context ⊂ harness ⊂ loop ⊂ graph; none deprecates the one below. Labels graph engineering "emerging and contested". · `solid` for the layering; `secondary` for its coinage table.
  - Spec called it "Beyond the Harness"; actual title is above.
- **[S15] The State of Context Engineering** (Drew Breunig) · https://www.dbreunig.com/2026/10/24/the-state-of-context-engineering.html · 2026-10-24 · "We're moving more and more of the work of assembling context from probabilistic model calls to deterministic code." Traces prompt (2023) → context → harness ("as we moved more of these tools out of the context and into code, we started calling it harness engineering"). Names "context inertia"; resetting context is often the fix. · `solid`

## Added (newer / primary, found during verification)

- **[S16] Extreme Harness Engineering for Token Billionaires — Ryan Lopopolo** (Latent Space podcast) · https://latent.space/p/harness-eng · 2026 (Apr 7) · Interview with [S1]'s author: >1M lines in repo, ~1,500 PRs, five months, zero manually written code; started as a team of three. · `solid` (for [S1] numbers)
- **[S17] Effective harnesses for long-running agents** (Justin Young, Anthropic) · https://www.anthropic.com/engineering/effective-harnesses-for-long-running-agents · 2025-11-26 · Initializer agent sets up the environment; later coding sessions do one feature at a time, commit to git, and update a progress file; a JSON feature list (all initially failing) stops premature "done" claims. · `solid`
- **[S18] Building effective agents** (Erik S., Barry Zhang, Anthropic) · https://www.anthropic.com/engineering/building-effective-agents · 2024-12-19 · Workflows = predefined code paths; agents = LLM directs its own process. Patterns: prompt chaining, routing, parallelization, orchestrator-workers, evaluator-optimizer. · `solid` (graph patterns pre-date "graph engineering")
- **[S19] Effective context engineering for AI agents** (Rajasekaran et al., Anthropic) · https://www.anthropic.com/engineering/effective-context-engineering-for-ai-agents · 2025-09-29 · Context is finite ("context rot", attention budget); just-in-time retrieval; long-horizon techniques: compaction, structured note-taking, sub-agents. · `solid`
- **[S20] My AI Adoption Journey** (Mitchell Hashimoto) · https://mitchellh.com/writing/my-ai-adoption-journey · 2026-02-05 · "I've grown to calling this 'harness engineering'" (fixing a problem once so the agent doesn't repeat it); "I don't need to invent any new terms here; if another one exists, I'll jump on the bandwagon." · `solid` (term timeline)
- **[S21] Three Key Loops for Building Great Software** (Andrew Ng, The Batch) · https://www.deeplearning.ai/the-batch/three-key-loops-for-building-great-software · 2026-06-26 · Primary source for the three loops (see below). · `solid`

## Secondary (color only)

- **[V1] Graph engineering: the frameworks that did it first** (Ayesha Aamir, Data Science Dojo) · https://datasciencedojo.com/blog/graph-engineering-frameworks/ · 2026-07-21 · Graph frameworks (LangGraph, Microsoft Agent Framework, Google ADK, CrewAI, LlamaIndex Workflows, OpenAI Agents SDK) pre-date the label; most workflows don't need a graph. · `secondary`
- **[V2] Loop Engineering, Harness Engineering, Context Engineering: what's the difference? (1/6)** (Benjamin Font Pera, codecentric) · https://www.codecentric.de/en/knowledge-hub/blog/loop-harness-context-engineering-explained · 2026-07-05 · Layers build context → harness → loop; weaknesses in a lower layer get amplified by repetition. Harness split into "guides" (before generation) and "sensors" (after). · `secondary`
- **[V3] Loop Engineering vs Prompt Engineering in 2026** (Lyzr) · https://www.lyzr.ai/blog/loop-engineering-vs-prompt-engineering/ · 2026-08-06 · Loops usually fail at the stop condition; a separate checker, not the worker agent, should judge completion. Vendor piece; its statistics are unverified. · `secondary`
- **[V4] Loop Engineering vs Harness Engineering** (MindStudio) · https://www.mindstudio.ai/blog/loop-engineering-vs-harness-engineering · 2026-06-18 · "The loop is the agent's behavior. The harness is the agent's environment." · `secondary`
- **[V5] Simon Willison — tag: tobias-lutke** · https://simonwillison.net/tags/tobias-lutke/ · 2025-06-27 entry · Records Lütke preferring "context engineering" over "prompt engineering", amplified by Karpathy. · `secondary` (context-engineering lineage)

- **[S27] Code Generation with AlphaCodium: From Prompt Engineering to Flow Engineering** (Ridnik, Kredo, Friedman, CodiumAI) · https://arxiv.org/abs/2401.08500 · 2024-01 · Coins "flow engineering": a test-based, multi-stage, iterative generate-run-fix flow; GPT-4 pass@5 on CodeContests validation 19% → 44% vs a single prompt. Predates the 2026 harness/loop/graph labels. · solid

## Dropped

- **Atlan** — no Atlan article on harness/loop/graph engineering could be located.
- **Faros** — only product/FAQ pages (harness listed as a "pillar"); no citable article.
- **Requesty** — no matching article found.
- **ADTmag** "Loop engineering emerges as developers put AI coding agents on repeat" (2026-07-01) — HTTP 403; content could not be verified.

## Three loops

**Primary source: [S21]** Andrew Ng, *The Batch*, 2026-06-26 (letter signed "Andrew"):
1. **Agentic coding loop** — minutes ("every few minutes, the coding agent might build and test a new version").
2. **Developer feedback loop** — "between tens of minutes and hours".
3. **External feedback loop** — users/alpha testers/A-B tests; "rarely taking less than hours and sometimes taking days or even weeks".
Ng frames these for building 0-to-1 products; the same letter notes "loop engineering" was already a buzzphrase after Boris Cherny's remarks went viral. Writers: attribute to Ng, dated June 2026; give timescales as above (vendor blogs simplify the outer loop to "days").

## Claude Code features (checked 2026-10-28)

- **`/loop`** — exists (as of 2026-10-28) · https://code.claude.com/docs/en/scheduled-tasks · Bundled skill that re-runs a prompt on a fixed interval or a self-paced interval inside an open session; recurring tasks expire after 7 days; bare `/loop` runs a built-in maintenance prompt (or `loop.md`).
- **Scheduled cloud tasks (formerly "routines")** — exists (as of 2026-10-28; research preview) · https://code.claude.com/docs/en/routines · Saved prompt + repos + connectors run on managed cloud infra with schedule, API or GitHub-event triggers; created via web UI or `/schedule`; minimum schedule interval 1 hour. Also: Desktop scheduled tasks and GitHub Actions.
- **Hooks** — exists (as of 2026-10-28) · https://code.claude.com/docs/en/hooks · User-defined handlers on lifecycle events (`SessionStart`, `UserPromptSubmit`, `PreToolUse`, `PostToolUse`, `PermissionRequest`, `Stop`, `SubagentStart`, `SessionEnd`, …); a `PreToolUse` hook can deny a tool call (exit code 2 or `permissionDecision: "deny"`).
- **Skills** — exists (as of 2026-10-28) · https://code.claude.com/docs/en/skills · Descriptions sit in context; the full `SKILL.md` body loads only when invoked (progressive disclosure); supporting files load on demand.
- **Subagents** — exists (as of 2026-10-28) · https://code.claude.com/docs/en/sub-agents · Each runs in its own context window with its own system prompt, tool allow/deny list (`tools`, `disallowedTools`) and `permissionMode`.

## Term timeline

| Term | Date | Who / where | Conflicting / competing claims |
|---|---|---|---|
| prompt engineering | ~2022–2023 | Community | [S15] dates the practice to 2023; [S14] says 2022–2024. |
| context engineering | Jun 2025 | Tobi Lütke post, amplified by Andrej Karpathy ([V5]); Anthropic formalizes Sep 2025 ([S19]) | [S14] credits Lütke + Karpathy + Anthropic. Exact tweet dates unverified (secondary only). |
| harness engineering | 2026-02-05 | Mitchell Hashimoto names it in a blog post ([S20]), disclaiming invention | OpenAI puts it in a title 2026-02-11 ([S1]) and is often credited as origin. [S14] says Hashimoto "crystallized" it, OpenAI adopted days later. Geoffrey Huntley's "Ralph" loop pattern predates the term ([S14], secondary). |
| loop engineering | Jun 2026 | Boris Cherny: "I don't prompt Claude anymore. I write loops that prompt Claude." Addy Osmani essay "Loop Engineering" 2026-06-07 names it ([V2], [V3]) | Steinberger also credited with a viral post ([V3]); [S14] credits Cherny + Osmani + IBM; Ng writes on it 2026-06-26 ([S21]); IBM formal definition 2026-07-17 ([S12]); academic review Aug 2026 ([S6]). Osmani's essay itself not fetched — cite via [V2]. |
| graph engineering | 2026-07-17/18 | Peter Steinberger tweet "Are we still talking loops or did we shift to graphs yet?" ([V1]; date differs by time zone) | LangChain: "not new, we've been doing it for three years" ([S13], 2026-07-22). [S14] marks origin "Community (contested)" and notes collision with older knowledge-graph usage. Workflow-graph research pre-dates it ([S7] Mar 2026; [S18] Dec 2024). Tweet itself not fetched. |

## Key facts for writers

1. "Agent = Model + Harness"; a harness is all code, config and execution logic that isn't the model. [S2]
2. The phrase "harness engineering" appears in Hashimoto's 2026-02-05 post, where he says he doesn't know an established term. [S20]
3. OpenAI's harness post (2026-02-11) reports ~1M lines and ~1,500 PRs over ~5 months with no hand-written code. [S1][S16]
4. A source-code study of 11 production coding-agent harnesses found none use a general-purpose agent framework or vector-embedding retrieval. [S3]
5. Automatically evolving a harness raised Terminal-Bench 2 pass@1 from 69.7% to 77.0%; tools, middleware and memory mattered more than the system prompt. [S4]
6. Moving rules from prompts into code made agents auditable; code-owned checks blocked violations without over-refusing, a bolt-on guardrail did not. [S9]
7. Keep the evaluator and permissions outside the loop that edits the harness, to limit reward hacking. [S11]
8. Context is finite: "context rot" degrades recall as the window grows; use compaction, notes and sub-agents. [S19]
9. Long-running agents benefit from a progress file, a JSON feature list that starts all-failing, and one feature per session with git commits. [S17]
10. Loop stages (IBM): goal → action → observation → adjustment. [S12]
11. There is no systematic evidence yet for loop engineering's claimed benefits; in 217 repos, loops rarely commit their state files. [S6]
12. Andrew Ng's three loops: agentic coding (minutes), developer feedback (tens of minutes–hours), external feedback (hours–weeks). [S21]
13. Graph nodes can be code, an LLM call, a tool call or a full agent; real agent graphs have cycles, so they're usually not DAGs. [S13]
14. Workflow patterns like orchestrator-workers and evaluator-optimizer were documented in Dec 2024, long before "graph engineering". [S18]
15. The terms nest — prompt ⊂ context ⊂ harness ⊂ loop ⊂ graph — and none replaces the layer below. [S14]
