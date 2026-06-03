# Design Spec — Diagrams for the Gen AI Guides (Mermaid → SVG pipeline + Zero-to-Hero pilot)

**Status:** Approved skeleton, pending user review of spec before plan
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-06-03
**Scope of THIS spec/plan:** the reusable diagram pipeline + the **Zero-to-Hero pilot** (~13 diagrams). Rolling the pipeline out to Hero-to-Architect and Architect-to-Leader is forward-looking context (§8), to be done as follow-on specs/plans once the pilot validates.
**Affected files:** `gen-ai-zero-to-hero-guide.html` (pilot); new `diagrams/` source tree and `scripts/` build tooling. Output HTML stays self-contained.

---

## 1. Purpose

All three Gen AI guides currently contain **zero true vector diagrams** (0 `<svg>`/`<canvas>`/Mermaid/`<img>`): every "diagram" is a CSS flexbox strip (`.pkt`), step-block (`.stp`), comparison columns (`.cmp`), or table. Readers don't perceive them as diagrams. This project adds genuine diagrams — schematic flowcharts/architecture, sequence diagrams, charts, and concept visuals — while preserving the guides' defining property: **self-contained single-file HTML, no runtime dependency, works offline.**

## 2. Approach (decided)

- **Author in Mermaid** (`.mmd` text files) — the maintainable source of truth, version-controlled.
- **Convert to SVG at dev-time** with `@mermaid-js/mermaid-cli` (`mmdc`); **embed the static SVG inline** in the HTML. No runtime JS for diagrams in the shipped file.
- **CSS-variable-driven theming:** post-process each SVG so its colors are the guides' CSS tokens (`var(--accent)`, `var(--surface2)`, `var(--border)`, `currentColor`, …). Diagrams then **re-theme automatically with the dark/light toggle**, like the rest of the guide.
- **Convert weak strips + add new:** upgrade the `.pkt` flow/spectrum strips that are really trying to be diagrams into proper SVG; add genuinely new diagrams/charts where missing; keep tables/step-blocks that already work.
- **Charts:** use Mermaid `xychart`/`pie` where adequate; use a small set of **hand-authored inline-SVG chart templates** where Mermaid's charting is too weak (bar charts, the embeddings scatter, the token-fill concept).
- **Sequence:** pilot one guide (Zero-to-Hero) end-to-end, validate in-browser (dark/light), then roll out.

**Tooling (decided):** Node v26, npm 11, npx, system Chrome present. Run mermaid-cli **on-demand via `npx -y @mermaid-js/mermaid-cli@<pinned x.y.z>` — no global install**. Pin the version: an unpinned `npx` pulls latest and can change rendered output, breaking the "reproduces exactly" goal. Set `PUPPETEER_EXECUTABLE_PATH` to the system Chrome (or a `puppeteer-config.json`) so puppeteer skips its Chromium download.

## 3. Pipeline architecture

### 3.1 Source layout
```
diagrams/
  _theme.json                 # mermaid config: theme=base + themeVariables set to SENTINEL hexes
  _lib/                        # reusable inline-SVG templates: bar-chart, timeline, scatter, token-fill
  zero-to-hero/
    family-tree.mmd
    neural-network.mmd
    training-pipeline.mmd
    transformer.mmd
    rag-pipeline.mmd
    agent-loop.mmd
    mcp-architecture.mmd
    tool-use.mmd              # sequenceDiagram
    context-window-sizes.chart.json   # data for the _lib bar-chart template (see 3.4)
    pricing.chart.json
    history-timeline.mmd
    embeddings-space.svg.json   # data for the _lib scatter template
    token-fill.svg.json         # data for the _lib token-fill template
scripts/
  build-diagrams.mjs          # render + post-process + inject (idempotent)
  lib/theme-map.mjs           # SENTINEL hex -> var(--token) replacement table
```

### 3.2 Build/inject flow (`scripts/build-diagrams.mjs`, Node, dev-time only)
For each `diagrams/<guide>/<id>.mmd`:
1. Run `mmdc -i <id>.mmd -o <id>.raw.svg -c diagrams/_theme.json` (+ `PUPPETEER_EXECUTABLE_PATH` to system Chrome, or a `puppeteer-config.json`).
2. **Post-process** `<id>.raw.svg`: (a) replace every SENTINEL hex with its `var(--token)` per `theme-map.mjs`; (b) strip width/height for responsive scaling, keep `viewBox`, add `max-width:100%;height:auto`; (c) add `role="img"` + `<title>`/`<desc>` for accessibility; (d) drop the `<style>` block `mmdc` inlines or rewrite its colors via the same map; (e) **uniquify all ids + references** — prefix every `id="…"`, `url(#…)`, and `href="#…"` with a per-diagram prefix and **assign them deterministically** (replace mmdc's generated/possibly-random ids with stable `d_<id>__N` counters). This is mandatory: inlining many Mermaid SVGs in one document otherwise produces duplicate ids (invalid HTML) and `url(#arrowhead)`/marker references that resolve to the *first* match in the document — arrowheads get mis-applied or vanish. Deterministic ids also keep re-runs byte-stable; (f) scope/rewrite the injected `<style>` selectors to the per-diagram prefix so styles don't bleed across diagrams. **Post-condition: the SVG has zero hardcoded hex colors and zero document-global ids.**
3. **Inject** the themed SVG into `gen-ai-<guide>-guide.html` between markers:
   `<!--DIAGRAM:<id>-->` … `<!--/DIAGRAM:<id>-->` (idempotent: replaces prior content between markers; re-runnable any time). The script **errors** if a `.mmd`/data file has no matching marker pair, and **warns** on markers with no source — no silent skips.
- Inline-SVG chart/concept diagrams skip mmdc — a small `*.chart.json`/`*.svg.json` data file feeds a reusable template in `diagrams/_lib/`, producing an SVG fragment (already using `var(--token)`) injected the same way.
- The script is **idempotent and reproducible**: editing a `.mmd` and re-running regenerates exactly that diagram in place. The `.mmd` is source; the HTML SVG is generated product.

### 3.3 Theming (CSS-variable-driven)
- `_theme.json` sets Mermaid `theme: "base"` with `themeVariables` pinned to a controlled **sentinel palette** — distinctive hexes chosen to be unambiguous to find-replace (e.g., `#111801` for background, `#222802` for surface2, `#333803` for border, `#444804` for accent/primary, `#555805` for text, etc.). **Pin _every_ relevant themeVariable, including the derived ones** Mermaid would otherwise compute itself (`primaryBorderColor`, `lineColor`, `secondaryColor`, `tertiaryColor`, `clusterBkg`, `edgeLabelBackground`, font/text colors, …) — otherwise Mermaid lightens/darkens a base color into an off-palette shade the sentinel map won't catch. As a safety net, add a CSS catch-all on `.diagram svg` (default `fill`/`stroke` → tokens), and on diagram #1 **grep the output SVG for any `#` hex that isn't a sentinel** before authoring the rest.
- `theme-map.mjs` maps each sentinel hex → the guide CSS token: `#444804 → var(--accent)`, `#222802 → var(--surface2)`, `#333803 → var(--border)`, `#555805 → currentColor`, etc. (Mermaid needs real colors at render time, so we render with sentinels then swap.)
- Result: the embedded SVG references only `var(--…)`/`currentColor`, so it follows the existing `:root` / `body.light` token flip — diagrams re-theme with the toggle, no per-mode duplication.
- The theme-map is **per guide** and must be validated against that guide's actual `:root` tokens before authoring. **Verified gotcha:** Zero-to-Hero (the pilot) defines **no `--accent` token** — its accent-role mapping must target an existing token there (`--blue`, `--cyan`, `--teal`, or `--purple` are all defined), or add an `--accent` token to that guide as the first step. (Parts 2 & 3 *do* define `--accent`.) Part 1's full `:root` set: `--bg, --surface, --surface2, --surface3, --border, --border-light, --text, --text-dim, --text-muted, --blue, --cyan, --teal, --green, --orange, --red, --pink, --purple, --yellow`. Node/edge/label colors map to existing tokens so diagrams look native to each guide.

### 3.4 Charts
- **Bar charts** (context-window sizes, pricing): hand-authored inline-SVG template (`render bars from a small data array`) using the guide's accent token (e.g. `var(--blue)` in Part 1, see §3.3) / `var(--surface2)` — Mermaid `xychart` is too limited/unstyled. Template lives in `diagrams/_lib` and is parameterized per use. **The bars must carry their exact values as labels** (e.g. "200K", "$3 / $15 per M") so converting a table to a chart loses no precision; keep the source table if a value is hard to read off the chart.
- **Timeline** (AI history): Mermaid `timeline` if it themes acceptably, else an inline-SVG horizontal timeline template.
- **Concept visuals** (embeddings scatter, token-fill): inline-SVG templates (not Mermaid).

### 3.5 Markup wrapper
Each injected diagram sits in `<figure class="diagram"><!--DIAGRAM:id-->…<!--/DIAGRAM:id--><figcaption class="ann">caption</figcaption></figure>`. A small `.diagram { ... }` CSS rule (centered, max-width, margin) is added once to the guide's `<style>`. Captions are authored by hand in the HTML (the script only manages the SVG between markers).

## 4. Pilot inventory — Zero-to-Hero (~13)

| # | Diagram | Type | Source | Section (approx) |
|---|---------|------|--------|------------------|
| 1 | AI / ML / DL family tree (nested) | flowchart | convert "Russian dolls" strip | family-tree |
| 2 | Neural network layers | flowchart | new / convert | neural-networks |
| 3 | Training pipeline (pre-train → fine-tune → RLHF) | flowchart | new | RLHF section |
| 4 | Transformer / attention (high-level) | flowchart | new | LLM section |
| 5 | RAG pipeline | flowchart | convert strip | RAG section |
| 6 | The agent loop (plan→act→observe) | flowchart (cycle) | convert strip | agents section |
| 7 | MCP architecture (host/client ↔ servers) | flowchart | new | MCP/config |
| 8 | Tool use (user→model→tool→model) | sequenceDiagram | new | tool-use |
| 9 | Context-window sizes | bar chart (inline-SVG) | convert table | tokens/context |
| 10 | Pricing comparison | bar chart (inline-SVG) | convert table | pricing |
| 11 | AI history timeline | timeline | convert/enhance | history |
| 12 | Embeddings semantic space | inline-SVG concept | new | embeddings |
| 13 | Tokens & context window filling | inline-SVG concept | new | tokens |

Covers every pipeline type (flowchart, sequence, chart, timeline, concept), validating the full pattern library. The plan pins exact insertion markers and convert-vs-new per item.

## 5. Non-goals

- **No runtime dependency in the shipped HTML** — no CDN Mermaid, no client-side rendering. The `.html` must still open offline with diagrams intact.
- **No full build system for the guides** — the guides remain hand-editable single files; the diagram script only manages content between `<!--DIAGRAM-->` markers.
- **Not converting every strip/table** — only the diagram-worthy ones; good tables/step-blocks stay.
- **Not re-reviewing guide prose** — this is a visual-layer project; content stays as-is except where a strip is replaced by an equivalent diagram.
- Parts 2 & 3 are **out of scope for this plan** (follow-on).

## 6. Success criteria

- Zero-to-Hero renders ~13 genuine diagrams that **re-theme correctly in both dark and light** mode.
- The shipped `gen-ai-zero-to-hero-guide.html` remains a single self-contained file that opens offline with all diagrams visible (no network, no JS errors).
- Re-running `scripts/build-diagrams.mjs` reproduces the embedded SVGs **byte-for-byte** from the `.mmd`/templates (idempotent — no git churn); editing a `.mmd` updates only its diagram.
- HTML integrity holds: tag balance, nav/section parity, no broken cross-links.
- The assembled HTML has **no duplicate element ids**, and every diagram's `url(#…)`/`href="#…"` resolves within itself (verify: no id appears twice document-wide).
- The pipeline + theme map + chart/concept templates are reusable as-is for Parts 2 & 3.

## 7. Risks & mitigations

- **SVG theming fidelity** — Mermaid can derive off-palette shades from base variables, so sentinel find-replace is *not* automatically exhaustive; mitigate by pinning *all* themeVariables (incl. derived) + a `.diagram svg` CSS catch-all, and verify on diagram #1 by grepping the output for any non-sentinel `#` hex before authoring the rest.
- **ID/marker collisions** — inlining many Mermaid SVGs in one document collides element ids and `url(#…)` marker refs; the §3.2 uniquify-and-prefix step is mandatory and is checked by the no-duplicate-id success criterion (§6).
- **mmdc/Chrome in this env** — point puppeteer at the system Chrome (`PUPPETEER_EXECUTABLE_PATH`) or a `puppeteer-config.json`; fall back to puppeteer's bundled Chromium.
- **File-size bloat** — inline SVGs are small (KB), far lighter than inlining Mermaid (~MB); acceptable.
- **Accessibility** — every diagram gets `role="img"` + `<title>`/`<desc>`; the prose around it still conveys the same content (diagrams augment, not replace, the explanation).
- **Spell-check noise** — domain terms (`mmdc`, `xychart`, `RLHF`, `agentic`, `gitlog`, `flexbox`) trip cSpell; add a small `cspell.json` words list so spec/HTML edits stay clean. Cosmetic.
- **Responsive/scroll** — strip fixed width, keep `viewBox`, `max-width:100%`; wrap wide diagrams in an `overflow-x:auto` container.

## 8. Rollout (forward-looking, separate plans)

- **Hero-to-Architect:** agentic-loop architecture, context-as-working-memory, tool blast-radius, multi-agent/parallel orchestration, MCP server internals, the `gitlog-report` worked-example flow, cost/model-routing chart.
- **Architect-to-Leader:** upgrade the recently-added strips into proper SVG (5-stage maturity model, CI/CD pipeline, incident-response sequence, config layering, threat surface) + cost/FinOps charts.
- Each reuses `diagrams/<guide>/` + the same scripts/theme-map/templates, and ends with the standard integrity + super-review gates.

## 9. Build approach (for the plan)

Implementation order (pilot): pin the mermaid-cli version + add `puppeteer-config.json` (run via `npx -y @mermaid-js/mermaid-cli@<pin>`, no global install; `PUPPETEER_EXECUTABLE_PATH`→system Chrome) → confirm Zero-to-Hero's `:root` token set + decide the accent-role mapping (no `--accent` there) → author `_theme.json` (all themeVariables sentinel-pinned) + `theme-map.mjs` + `build-diagrams.mjs` (incl. the id/marker uniquify+deterministic step) → prove the full pipeline on ONE diagram (agent-loop), validate dark/light in-browser AND grep for stray hex + duplicate ids → author the remaining 12 `.mmd`/templates → place `<figure>`+markers in the HTML and inject → integrity check (tag balance, nav parity, no dup ids) → visual review (dark/light) → super-review. One commit per logical step.
