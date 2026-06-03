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

**Tooling confirmed available:** Node v26, npm 11, npx, system Chrome present; `mmdc` installs via `@mermaid-js/mermaid-cli`.

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
2. **Post-process** `<id>.raw.svg`: (a) replace every SENTINEL hex with its `var(--token)` per `theme-map.mjs`; (b) strip width/height for responsive scaling, keep `viewBox`, add `max-width:100%;height:auto`; (c) add `role="img"` + `<title>`/`<desc>` for accessibility; (d) drop the `<style>` block `mmdc` inlines or rewrite its colors via the same map.
3. **Inject** the themed SVG into `gen-ai-<guide>-guide.html` between markers:
   `<!--DIAGRAM:<id>-->` … `<!--/DIAGRAM:<id>-->` (idempotent: replaces prior content between markers; re-runnable any time).
- Inline-SVG chart/concept diagrams skip mmdc — a small `*.chart.json`/`*.svg.json` data file feeds a reusable template in `diagrams/_lib/`, producing an SVG fragment (already using `var(--token)`) injected the same way.
- The script is **idempotent and reproducible**: editing a `.mmd` and re-running regenerates exactly that diagram in place. The `.mmd` is source; the HTML SVG is generated product.

### 3.3 Theming (CSS-variable-driven)
- `_theme.json` sets Mermaid `theme: "base"` with `themeVariables` pinned to a controlled **sentinel palette** — distinctive hexes chosen to be unambiguous to find-replace (e.g., `#111801` for background, `#222802` for surface2, `#333803` for border, `#444804` for accent/primary, `#555805` for text, etc.).
- `theme-map.mjs` maps each sentinel hex → the guide CSS token: `#444804 → var(--accent)`, `#222802 → var(--surface2)`, `#333803 → var(--border)`, `#555805 → currentColor`, etc. (Mermaid needs real colors at render time, so we render with sentinels then swap.)
- Result: the embedded SVG references only `var(--…)`/`currentColor`, so it follows the existing `:root` / `body.light` token flip — diagrams re-theme with the toggle, no per-mode duplication.
- The guide's existing accent token (Part 1 default) is reused; node/edge/label colors map to existing tokens so diagrams look native to each guide.

### 3.4 Charts
- **Bar charts** (context-window sizes, pricing): hand-authored inline-SVG template (`render bars from a small data array`) using `var(--accent)`/`var(--surface2)` — Mermaid `xychart` is too limited/unstyled. Template lives in `diagrams/_lib` and is parameterized per use.
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
- Re-running `scripts/build-diagrams.mjs` reproduces the embedded SVGs exactly from the `.mmd`/templates (idempotent); editing a `.mmd` updates only its diagram.
- HTML integrity holds: tag balance, nav/section parity, no broken cross-links.
- The pipeline + theme map + chart/concept templates are reusable as-is for Parts 2 & 3.

## 7. Risks & mitigations

- **SVG theming fidelity** — sentinel-palette + deterministic find-replace avoids guessing Mermaid's output colors; validate the map on the first diagram before authoring all 13.
- **mmdc/Chrome in this env** — point puppeteer at the system Chrome (`PUPPETEER_EXECUTABLE_PATH`) or a `puppeteer-config.json`; fall back to puppeteer's bundled Chromium.
- **File-size bloat** — inline SVGs are small (KB), far lighter than inlining Mermaid (~MB); acceptable.
- **Accessibility** — every diagram gets `role="img"` + `<title>`/`<desc>`; the prose around it still conveys the same content (diagrams augment, not replace, the explanation).
- **Responsive/scroll** — strip fixed width, keep `viewBox`, `max-width:100%`; wrap wide diagrams in an `overflow-x:auto` container.

## 8. Rollout (forward-looking, separate plans)

- **Hero-to-Architect:** agentic-loop architecture, context-as-working-memory, tool blast-radius, multi-agent/parallel orchestration, MCP server internals, the `gitlog-report` worked-example flow, cost/model-routing chart.
- **Architect-to-Leader:** upgrade the recently-added strips into proper SVG (5-stage maturity model, CI/CD pipeline, incident-response sequence, config layering, threat surface) + cost/FinOps charts.
- Each reuses `diagrams/<guide>/` + the same scripts/theme-map/templates, and ends with the standard integrity + super-review gates.

## 9. Build approach (for the plan)

Implementation order (pilot): install `mmdc` + add `puppeteer-config.json` → author `_theme.json` + `theme-map.mjs` + `build-diagrams.mjs` → prove the full pipeline on ONE diagram (agent-loop) and validate dark/light in-browser → author the remaining 12 `.mmd`/templates → place `<figure>`+markers in the HTML and inject → integrity check → visual review (dark/light) → super-review. One commit per logical step.
