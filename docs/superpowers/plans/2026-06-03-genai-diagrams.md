# Gen AI Guide Diagrams — Pipeline + Zero-to-Hero Pilot — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a dev-time Mermaid→SVG→inline pipeline (CSS-variable themed, self-contained output) and use it to add ~13 genuine diagrams to `gen-ai-zero-to-hero-guide.html`.

**Architecture:** Author diagrams as Mermaid `.mmd` (and small `*.json` data files for charts/concepts) under `diagrams/`. A Node script renders each via `npx @mermaid-js/mermaid-cli` (pinned), post-processes the SVG (recolor sentinel hexes → `var(--token)`, uniquify ids/marker refs, scope the `<style>` block, make responsive + accessible), and injects it inline into the guide HTML between `<!--DIAGRAM:id-->` markers. Shipped HTML keeps zero runtime dependency and works offline. Inline-SVG templates handle charts/concepts Mermaid renders poorly.

**Tech Stack:** Node v26 (ESM `.mjs`), `@mermaid-js/mermaid-cli` via `npx` (pinned, no global install), system Chrome via `PUPPETEER_EXECUTABLE_PATH`, vanilla SVG. Source spec: `docs/superpowers/specs/2026-06-03-genai-diagrams-design.md`.

---

## File Structure

- Create:
  - `puppeteer-config.json` — points mermaid-cli's puppeteer at system Chrome
  - `cspell.json` — domain-term word list (kills spell-check noise)
  - `diagrams/_theme.json` — Mermaid config: `theme: base` + all `themeVariables` pinned to sentinel hexes
  - `scripts/lib/theme-map.mjs` — sentinel-hex → `var(--token)` table (Zero-to-Hero mapping)
  - `scripts/lib/postprocess.mjs` — SVG post-process (recolor, uniquify ids, scope style, responsive, a11y)
  - `scripts/lib/templates.mjs` — inline-SVG generators: `barChart`, `timeline`, `scatter`, `tokenFill`
  - `scripts/build-diagrams.mjs` — orchestrator: render + post-process + inject (idempotent)
  - `diagrams/zero-to-hero/*.mmd` (8 Mermaid) + `*.chart.json` / `*.svg.json` (5 data files)
- Modify:
  - `gen-ai-zero-to-hero-guide.html` — add `.diagram`/`figure` CSS once; add `<figure>`+`<!--DIAGRAM:id-->` markers at 13 spots; receive injected SVGs

No package.json is added (npx-on-demand). The guides stay hand-editable single files; the script only manages content between markers.

---

## Conventions used in this plan

- **Run mermaid-cli only via** `npx -y @mermaid-js/mermaid-cli@11.4.2 ...` (pinned — do not use a floating version; reproducibility depends on it). `PUPPETEER_EXECUTABLE_PATH` is set to system Chrome.
- **Sentinel palette:** Mermaid is rendered with deliberately-distinctive hex colors (`#f0a001`…`#f0a00a`) that the post-process find-replaces with `var(--token)`. These hexes are chosen to never collide with real content.
- **Per-diagram id prefix:** every diagram's SVG gets all `id`/`url(#…)`/`href="#…"` rewritten to `d-<id>-nN` (deterministic counter), so multiple inlined SVGs never collide.
- **Marker convention:** in the HTML, a diagram lives in
  `<figure class="diagram"><!--DIAGRAM:<id>--><!--/DIAGRAM:<id>--><figcaption class="ann">caption</figcaption></figure>` — the script replaces only the text between the two markers.
- **Verification per diagram (the "test"):** after injecting, the SVG must have (a) zero raw `#` hex colors, (b) zero ids duplicated elsewhere in the document, (c) a `viewBox` and no fixed width/height; and the page's `<div>` balance is unchanged. Commands given per task.
- **Commit convention:** infra tasks `feat(diagrams): …`; diagram-authoring tasks `docs(zero-to-hero): add diagram — …`; fixes `fix(diagrams): …`.
- **The pinned version `11.4.2` is illustrative** — Task 1 step 2 resolves and records the actual latest stable; use that exact pin everywhere after.

---

## Task 1: Tooling setup + config + token decision

**Files:** Create `puppeteer-config.json`, `cspell.json`. Read `gen-ai-zero-to-hero-guide.html` (token set only).

- [ ] **Step 1: Confirm Chrome path**

```bash
ls "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" && echo OK
```
Expected: the binary path prints + `OK`.

- [ ] **Step 2: Resolve and record the mermaid-cli pin, and smoke-test it against system Chrome**

```bash
cd /Users/divakaran/arrcus_workspace/guides
cat > puppeteer-config.json <<'EOF'
{ "executablePath": "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome", "args": ["--no-sandbox"] }
EOF
printf 'flowchart LR\n A[hello] --> B[world]\n' > /tmp/smoke.mmd
npx -y @mermaid-js/mermaid-cli@11.4.2 -p puppeteer-config.json -i /tmp/smoke.mmd -o /tmp/smoke.svg
head -c 80 /tmp/smoke.svg; echo
```
Expected: `npx` downloads mermaid-cli@11.4.2 on first run, then produces `/tmp/smoke.svg` starting with `<svg ...`. If `11.4.2` errors as not-found, run `npm view @mermaid-js/mermaid-cli version`, use that exact version, and **update every `@11.4.2` in this plan to the resolved pin**.

- [ ] **Step 3: Record Zero-to-Hero's `:root` token set and pick the accent-role token**

```bash
awk '/^[[:space:]]*:root[[:space:]]*\{/,/\}/' gen-ai-zero-to-hero-guide.html | grep -oE '\-\-[a-z0-9-]+:' | sort -u
```
Expected set includes: `--bg --surface --surface2 --surface3 --border --border-light --text --text-dim --text-muted --blue --cyan --teal --green --orange --red --pink --purple --yellow`. **There is no `--accent`** — the accent role maps to `--blue` (decided in the spec).

- [ ] **Step 4: Add cspell word list**

```bash
cat > cspell.json <<'EOF'
{ "version": "0.2", "words": ["mmdc","xychart","RLHF","agentic","gitlog","flexbox","viewBox","puppeteer","Mermaid","mermaid","themeVariables","currentColor","Genie","pyATS","arcOS","SRv6","EVPN"] }
EOF
```

- [ ] **Step 5: Commit**

```bash
git add puppeteer-config.json cspell.json
git commit -m "feat(diagrams): tooling config (puppeteer->system Chrome, cspell words)"
```

---

## Task 2: Sentinel theme + theme-map

**Files:** Create `diagrams/_theme.json`, `scripts/lib/theme-map.mjs`.

- [ ] **Step 1: Write `diagrams/_theme.json` (all themeVariables pinned to sentinels)**

```json
{
  "theme": "base",
  "themeVariables": {
    "background": "#f0a001",
    "primaryColor": "#f0a002",
    "primaryBorderColor": "#f0a003",
    "primaryTextColor": "#f0a004",
    "lineColor": "#f0a005",
    "secondaryColor": "#f0a006",
    "secondaryBorderColor": "#f0a003",
    "secondaryTextColor": "#f0a004",
    "tertiaryColor": "#f0a007",
    "tertiaryBorderColor": "#f0a003",
    "tertiaryTextColor": "#f0a004",
    "mainBkg": "#f0a002",
    "nodeBorder": "#f0a003",
    "clusterBkg": "#f0a001",
    "clusterBorder": "#f0a003",
    "edgeLabelBackground": "#f0a001",
    "textColor": "#f0a004",
    "nodeTextColor": "#f0a004",
    "actorBkg": "#f0a002",
    "actorBorder": "#f0a003",
    "actorTextColor": "#f0a004",
    "signalColor": "#f0a005",
    "signalTextColor": "#f0a004",
    "labelBoxBkgColor": "#f0a002",
    "labelBoxBorderColor": "#f0a003",
    "labelTextColor": "#f0a004",
    "noteBkgColor": "#f0a006",
    "noteBorderColor": "#f0a003",
    "noteTextColor": "#f0a004",
    "fontFamily": "inherit"
  }
}
```

- [ ] **Step 2: Write `scripts/lib/theme-map.mjs`**

```javascript
// Sentinel hex -> guide CSS token. Zero-to-Hero has NO --accent; accent role = --blue.
export const THEME_MAP = {
  '#f0a001': 'var(--bg)',          // backgrounds / cluster / edge-label bg
  '#f0a002': 'var(--surface2)',    // node fill
  '#f0a003': 'var(--border)',      // borders
  '#f0a004': 'currentColor',       // text
  '#f0a005': 'var(--text-dim)',    // edges / signals
  '#f0a006': 'var(--blue)',        // accent role (Part 1)
  '#f0a007': 'var(--surface3)',    // tertiary fill
};
// Any sentinel family hex not mapped above (defensive): map to currentColor.
export const SENTINEL_RE = /#f0a00[0-9a-f]/gi;
```

- [ ] **Step 3: Commit**

```bash
git add diagrams/_theme.json scripts/lib/theme-map.mjs
git commit -m "feat(diagrams): sentinel theme config + theme-map (Part 1 tokens)"
```

---

## Task 3: Post-process module + build orchestrator (proven on agent-loop)

**Files:** Create `scripts/lib/postprocess.mjs`, `scripts/build-diagrams.mjs`, `diagrams/zero-to-hero/agent-loop.mmd`.

- [ ] **Step 1: Write `scripts/lib/postprocess.mjs`**

```javascript
import { THEME_MAP, SENTINEL_RE } from './theme-map.mjs';

// Make a raw mmdc SVG safe to inline: unique ids, themed colors, scoped <style>, responsive, a11y.
export function postprocess(rawSvg, id, title, desc = '') {
  let svg = rawSvg;
  const prefix = `d-${id}`;

  // 1) Collect every id="..." and assign a deterministic unique name.
  const idMap = new Map();
  let n = 0;
  svg = svg.replace(/\bid="([^"]+)"/g, (_, old) => {
    if (!idMap.has(old)) idMap.set(old, `${prefix}-n${n++}`);
    return `id="${idMap.get(old)}"`;
  });
  // 2) Rewrite references to those ids: url(#old), href="#old", xlink:href="#old".
  for (const [old, neu] of idMap) {
    const esc = old.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    svg = svg.replace(new RegExp(`url\\(#${esc}\\)`, 'g'), `url(#${neu})`);
    svg = svg.replace(new RegExp(`(href=")#${esc}(")`, 'g'), `$1#${neu}$2`);
  }

  // 3) Scope the inlined <style> so it can't bleed: prefix each selector with the root svg id.
  const rootId = idMap.get([...idMap.keys()][0]) || `${prefix}-root`;
  svg = svg.replace(/<style>([\s\S]*?)<\/style>/g, (_, css) => {
    const scoped = css.replace(/([^{}]+)\{/g, (m, sel) =>
      sel.split(',').map(s => `#${rootId} ${s.trim()}`).join(',') + '{');
    return `<style>${scoped}</style>`;
  });

  // 4) Recolor: every sentinel hex -> token. Then assert none remain.
  svg = svg.replace(SENTINEL_RE, (hex) => THEME_MAP[hex.toLowerCase()] || 'currentColor');

  // 5) Responsive + a11y on the root <svg>: drop fixed width/height, keep viewBox.
  svg = svg.replace(/<svg([^>]*?)>/, (m, attrs) => {
    let a = attrs
      .replace(/\swidth="[^"]*"/, '')
      .replace(/\sheight="[^"]*"/, '')
      .replace(/\sstyle="[^"]*"/, '');
    return `<svg${a} role="img" style="max-width:100%;height:auto"><title>${title}</title><desc>${desc || title}</desc>`;
  });

  return svg.trim();
}

// Verify a finished fragment is safe. Returns array of problems (empty = ok).
export function lintSvg(svg) {
  const problems = [];
  const hexes = svg.match(/#[0-9a-fA-F]{3,8}\b/g);
  if (hexes) problems.push(`raw hex colors remain: ${[...new Set(hexes)].join(', ')}`);
  if (/#f0a00/i.test(svg)) problems.push('unmapped sentinel remains');
  return problems;
}
```

- [ ] **Step 2: Write `scripts/build-diagrams.mjs`**

```javascript
#!/usr/bin/env node
// Render diagrams/<guide>/*.mmd (+ *.json) -> themed inline SVG -> inject into the guide HTML.
// ESM (.mjs); uses top-level await + dynamic import so the registry and templates
// load at runtime (static `import` cannot take a template-literal path, and templates.mjs
// may not exist yet when only Mermaid diagrams are built).
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, mkdtempSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { postprocess, lintSvg } from './lib/postprocess.mjs';

const PIN = '@mermaid-js/mermaid-cli@11.4.2';
const ROOT = '/Users/divakaran/arrcus_workspace/guides';
const HERE = dirname(fileURLToPath(import.meta.url));     // .../scripts
const GUIDE = process.argv[2] || 'zero-to-hero';
const ONLY = process.argv[3];                            // optional: build a single id
const HTML = join(ROOT, `gen-ai-${GUIDE}-guide.html`);
const DIR = join(ROOT, 'diagrams', GUIDE);
const CHROME = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';

// id -> { kind, title, desc?, data? } ; kind: 'mermaid' | 'barChart' | 'timeline' | 'scatter' | 'tokenFill'
const { REGISTRY } = await import(pathToFileURL(join(DIR, '_registry.mjs')).href);

function renderMermaid(id) {
  const out = join(mkdtempSync(join(tmpdir(), 'dg-')), `${id}.svg`);
  execFileSync('npx', ['-y', PIN, '-p', join(ROOT, 'puppeteer-config.json'),
    '-c', join(ROOT, 'diagrams', '_theme.json'),
    '-i', join(DIR, `${id}.mmd`), '-o', out],
    { stdio: 'inherit', env: { ...process.env, PUPPETEER_EXECUTABLE_PATH: CHROME } });
  return readFileSync(out, 'utf8');
}

async function build(id, meta) {
  let svg;
  if (meta.kind === 'mermaid') {
    svg = postprocess(renderMermaid(id), id, meta.title, meta.desc);
  } else {
    const tpl = await import(pathToFileURL(join(HERE, 'lib', 'templates.mjs')).href);
    const data = JSON.parse(readFileSync(join(DIR, meta.data), 'utf8'));
    svg = tpl[meta.kind](id, data, meta.title);          // templates emit prefixed/no ids + tokens
  }
  const problems = lintSvg(svg);
  if (problems.length) throw new Error(`[${id}] ${problems.join('; ')}`);
  return svg;
}

function inject(html, id, svg) {
  const open = `<!--DIAGRAM:${id}-->`, close = `<!--/DIAGRAM:${id}-->`;
  const i = html.indexOf(open), j = html.indexOf(close);
  if (i === -1 || j === -1) throw new Error(`marker for "${id}" not found in HTML (add <figure>+markers first)`);
  return html.slice(0, i + open.length) + '\n' + svg + '\n' + html.slice(j);
}

let html = readFileSync(HTML, 'utf8');
const ids = ONLY ? [ONLY] : Object.keys(REGISTRY);
for (const id of ids) html = inject(html, id, await build(id, REGISTRY[id]));
// warn on orphan markers (marker present in HTML, no registry entry)
for (const m of html.matchAll(/<!--DIAGRAM:([a-z0-9-]+)-->/g))
  if (!REGISTRY[m[1]]) console.warn(`WARN: marker "${m[1]}" has no registry entry`);
writeFileSync(HTML, html);
console.log(`injected ${ids.length} diagram(s) into ${HTML}`);
```

- [ ] **Step 3: Write the spike Mermaid `diagrams/zero-to-hero/agent-loop.mmd`**

```
flowchart LR
  P[Plan: decide next step] --> A[Act: call a tool]
  A --> O[Observe: read result]
  O --> D{Goal met?}
  D -- no --> P
  D -- yes --> E[Done]
```

- [ ] **Step 4: Create a minimal registry so the orchestrator runs (full registry in Task 4)**

```bash
cd /Users/divakaran/arrcus_workspace/guides
mkdir -p diagrams/zero-to-hero
cat > diagrams/zero-to-hero/_registry.mjs <<'EOF'
export const REGISTRY = {
  'agent-loop': { kind: 'mermaid', title: 'The agent loop: plan, act, observe, repeat' },
};
EOF
```

- [ ] **Step 5: Run the pipeline spike on agent-loop** (no guide edits yet — prove render + post-process)

Render + post-process the spike directly (does **not** touch the guide yet), then eyeball it in a themed harness page:

```bash
cd /Users/divakaran/arrcus_workspace/guides
node --input-type=module -e "
import { postprocess, lintSvg } from './scripts/lib/postprocess.mjs';
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';
execFileSync('npx',['-y','@mermaid-js/mermaid-cli@11.4.2','-p','puppeteer-config.json','-c','diagrams/_theme.json','-i','diagrams/zero-to-hero/agent-loop.mmd','-o','/tmp/agent-loop.raw.svg'],{stdio:'inherit',env:{...process.env,PUPPETEER_EXECUTABLE_PATH:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome'}});
const svg = postprocess(readFileSync('/tmp/agent-loop.raw.svg','utf8'),'agent-loop','The agent loop');
console.log('lint:', lintSvg(svg));
const ids=(svg.match(/ id=\"([^\"]+)\"/g)||[]); console.log('ids:',ids.length,'internal dups:',ids.filter((v,i)=>ids.indexOf(v)!==i));
writeFileSync('/tmp/agent-loop.svg', svg);
"
# eyeball in a themed harness:
printf '%s' '<!DOCTYPE html><html><head><style>:root{--bg:#0b0f17;--surface2:#1a2230;--surface3:#222c3c;--border:#2c3848;--text:#e6edf3;--text-dim:#9aa7b4;--blue:#4f9fff}body{color:var(--text);background:var(--bg);padding:40px}</style></head><body><figure class="diagram">' > /tmp/harness.html
cat /tmp/agent-loop.svg >> /tmp/harness.html
printf '%s' '</figure></body></html>' >> /tmp/harness.html
open /tmp/harness.html
```
Expected: `lint: []` (no raw hex, no sentinel), `internal dups: []`, and the harness shows the flowchart in dark colors with visible arrowheads.

- [ ] **Step 6: Iterate `postprocess.mjs` until `lint: []` and the diagram renders** (this is the de-risk task — fix regexes here, not later). Confirm: zero raw hex, every `id` starts with `d-agent-loop-`, arrowheads visible.

- [ ] **Step 7: Commit**

```bash
git add scripts/lib/postprocess.mjs scripts/build-diagrams.mjs diagrams/zero-to-hero/agent-loop.mmd diagrams/zero-to-hero/_registry.mjs
git commit -m "feat(diagrams): build pipeline + post-process, proven on agent-loop"
```

---

## Task 4: Guide CSS + first real embed (agent-loop) with dark/light validation

**Files:** Modify `gen-ai-zero-to-hero-guide.html`.

- [ ] **Step 1: Add the `.diagram` CSS once** (inside the guide's `<style>`, after the `.info` rules)

```css
figure.diagram { margin: 18px 0; text-align: center; }
figure.diagram svg { max-width: 100%; height: auto; }
figure.diagram .ann { margin-top: 6px; }
.diagram-scroll { overflow-x: auto; }
```

- [ ] **Step 2: Place the agent-loop figure + markers** in the AI-agents section. Find it: `grep -n "The Agent Loop\|agent loop\|<h2>" gen-ai-zero-to-hero-guide.html` near the agents section; insert after the relevant `<p>`:

```html
<figure class="diagram"><!--DIAGRAM:agent-loop--><!--/DIAGRAM:agent-loop-->
<figcaption class="ann">The agentic loop: the model plans, acts via a tool, observes the result, and repeats until the goal is met.</figcaption></figure>
```

- [ ] **Step 3: Inject just this diagram**

```bash
cd /Users/divakaran/arrcus_workspace/guides
node scripts/build-diagrams.mjs zero-to-hero agent-loop
```
Expected: `injected 1 diagram(s)`.

- [ ] **Step 4: Verify integrity + open in browser**

```bash
f=gen-ai-zero-to-hero-guide.html
echo "raw hex inside the injected svg:"; grep -o 'DIAGRAM:agent-loop-->[^!]*' "$f" | grep -oE '#[0-9a-fA-F]{3,8}' | head
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
```
Expected: no hex printed; div balance unchanged. Open the guide, go to the agents section, **toggle dark/light** — the diagram recolors with the theme. No console errors.

- [ ] **Step 5: Commit**

```bash
git add gen-ai-zero-to-hero-guide.html
git commit -m "docs(zero-to-hero): add diagram — agent loop (first embed, validated dark/light)"
```

---

## Task 5: Inline-SVG templates (`_lib`) for charts + concepts

**Files:** Create `scripts/lib/templates.mjs`.

- [ ] **Step 1: Write `scripts/lib/templates.mjs`** — deterministic, token-colored, prefixed ids, value labels.

```javascript
// All templates emit inline SVG using var(--token) only, with ids prefixed `d-<id>-`.
// Data shapes are documented at each function.

const esc = s => String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');

// barChart: { unit, bars:[{label, value, display}] }  value drives bar length; display is the printed number.
export function barChart(id, data, title) {
  const W = 720, rowH = 34, padL = 150, padR = 70, top = 10;
  const max = Math.max(...data.bars.map(b => b.value));
  const H = top * 2 + data.bars.length * rowH;
  const rows = data.bars.map((b, i) => {
    const y = top + i * rowH, w = Math.max(2, (b.value / max) * (W - padL - padR));
    return `<text x="${padL-8}" y="${y+rowH/2}" text-anchor="end" dominant-baseline="middle" fill="currentColor" font-size="13">${esc(b.label)}</text>`
      + `<rect x="${padL}" y="${y+6}" width="${w}" height="${rowH-14}" rx="3" fill="var(--blue)"/>`
      + `<text x="${padL+w+8}" y="${y+rowH/2}" dominant-baseline="middle" fill="var(--text-dim)" font-size="12">${esc(b.display ?? b.value)}</text>`;
  }).join('');
  return `<svg viewBox="0 0 ${W} ${H}" role="img" style="max-width:100%;height:auto" font-family="inherit">`
    + `<title>${esc(title)}</title><desc>${esc(data.unit||title)}</desc>${rows}</svg>`;
}

// timeline: { events:[{year, label}] } horizontal.
export function timeline(id, data, title) {
  const W = 760, H = 120, padX = 30, y = 60;
  const xs = (i) => padX + i * ((W - padX*2) / Math.max(1, data.events.length - 1));
  const line = `<line x1="${padX}" y1="${y}" x2="${W-padX}" y2="${y}" stroke="var(--border)" stroke-width="2"/>`;
  const pts = data.events.map((e, i) => {
    const x = xs(i);
    return `<circle cx="${x}" cy="${y}" r="5" fill="var(--blue)"/>`
      + `<text x="${x}" y="${y-14}" text-anchor="middle" fill="currentColor" font-size="12" font-weight="600">${esc(e.year)}</text>`
      + `<text x="${x}" y="${y+22}" text-anchor="middle" fill="var(--text-dim)" font-size="11">${esc(e.label)}</text>`;
  }).join('');
  return `<svg viewBox="0 0 ${W} ${H}" role="img" style="max-width:100%;height:auto" font-family="inherit"><title>${esc(title)}</title>${line}${pts}</svg>`;
}

// scatter: { clusters:[{label, color, points:[[x,y],...]}] }  x,y in 0..1 (semantic space concept).
export function scatter(id, data, title) {
  const W = 520, H = 360, pad = 24;
  const X = v => pad + v * (W - pad*2), Y = v => H - pad - v * (H - pad*2);
  const body = data.clusters.map(c => {
    const dots = c.points.map(([x,y]) => `<circle cx="${X(x).toFixed(1)}" cy="${Y(y).toFixed(1)}" r="6" fill="var(${c.color})" opacity="0.85"/>`).join('');
    const [lx,ly] = c.points[0];
    return dots + `<text x="${(X(lx)+10).toFixed(1)}" y="${(Y(ly)-8).toFixed(1)}" fill="currentColor" font-size="12">${esc(c.label)}</text>`;
  }).join('');
  return `<svg viewBox="0 0 ${W} ${H}" role="img" style="max-width:100%;height:auto" font-family="inherit"><title>${esc(title)}</title>`
    + `<rect x="1" y="1" width="${W-2}" height="${H-2}" rx="8" fill="none" stroke="var(--border)"/>${body}</svg>`;
}

// tokenFill: { total, segments:[{label,count,color}] } a single bar showing a context window filling up.
export function tokenFill(id, data, title) {
  const W = 720, H = 80, x0 = 10, y0 = 24, barW = W - 20, barH = 30;
  let x = x0;
  const segs = data.segments.map(s => {
    const w = (s.count / data.total) * barW;
    const r = `<rect x="${x.toFixed(1)}" y="${y0}" width="${w.toFixed(1)}" height="${barH}" fill="var(${s.color})"/>`
      + `<text x="${(x+w/2).toFixed(1)}" y="${y0+barH+16}" text-anchor="middle" fill="var(--text-dim)" font-size="11">${esc(s.label)}</text>`;
    x += w; return r;
  }).join('');
  return `<svg viewBox="0 0 ${W} ${H}" role="img" style="max-width:100%;height:auto" font-family="inherit"><title>${esc(title)}</title>`
    + `<rect x="${x0}" y="${y0}" width="${barW}" height="${barH}" rx="3" fill="var(--surface2)" stroke="var(--border)"/>${segs}</svg>`;
}
```

- [ ] **Step 2: Unit-check the templates render valid SVG with no raw hex**

```bash
cd /Users/divakaran/arrcus_workspace/guides
node --input-type=module -e "
import { barChart } from './scripts/lib/templates.mjs';
const s = barChart('t', {unit:'tokens', bars:[{label:'GPT-3.5',value:4096,display:'4K'},{label:'Gemini 3 Pro',value:1000000,display:'1M'}]}, 'Context windows');
console.log(/#[0-9a-fA-F]{3,8}/.test(s) ? 'FAIL raw hex' : 'ok no hex'); console.log(s.slice(0,60));
"
```
Expected: `ok no hex` + an `<svg …` prefix.

- [ ] **Step 3: Commit**

```bash
git add scripts/lib/templates.mjs
git commit -m "feat(diagrams): inline-SVG templates (bar, timeline, scatter, token-fill)"
```

---

## Task 6: Author the 6 remaining flowcharts + the sequence diagram (Mermaid)

**Files:** Create `diagrams/zero-to-hero/{family-tree,neural-network,training-pipeline,transformer,rag-pipeline,mcp-architecture,tool-use}.mmd`; update `_registry.mjs`; add markers in the HTML; inject.

- [ ] **Step 1: Write the 7 Mermaid sources** (one file each)

`family-tree.mmd`:
```
flowchart TB
  AI[Artificial Intelligence] --> ML[Machine Learning]
  ML --> DL[Deep Learning]
  DL --> GenAI[Generative AI / LLMs]
```
`neural-network.mmd`:
```
flowchart LR
  I1((in)) --> H1((●)) & H2((●)) & H3((●))
  I2((in)) --> H1 & H2 & H3
  H1 --> O1((out))
  H2 --> O1
  H3 --> O1
```
`training-pipeline.mmd`:
```
flowchart LR
  D[Raw text corpus] --> PT[Pre-training: predict next token]
  PT --> FT[Fine-tuning: task examples]
  FT --> RL[RLHF: human preference tuning]
  RL --> M[Helpful, aligned model]
```
`transformer.mmd`:
```
flowchart LR
  T[Tokens] --> E[Embeddings]
  E --> A[Self-attention: weigh related tokens]
  A --> FF[Feed-forward layers]
  FF --> P[Next-token probabilities]
```
`rag-pipeline.mmd`:
```
flowchart LR
  Q[User question] --> EM[Embed query]
  EM --> R[(Vector store)]
  R --> C[Retrieve top-k chunks]
  C --> AUG[Augment prompt with context]
  AUG --> G[LLM generates grounded answer]
```
`mcp-architecture.mmd`:
```
flowchart LR
  H[Host app e.g. Claude Code] --> CL[MCP client]
  CL <--> S1[MCP server: files]
  CL <--> S2[MCP server: database]
  CL <--> S3[MCP server: web]
  S1 -.exposes.-> T[tools / resources / prompts]
```
`tool-use.mmd`:
```
sequenceDiagram
  participant U as User
  participant M as Model
  participant T as Tool
  U->>M: question
  M->>T: tool call (args)
  T->>M: result
  M->>U: final answer
```

- [ ] **Step 2: Replace `_registry.mjs` with the full flowchart/sequence set**

```javascript
export const REGISTRY = {
  'agent-loop':        { kind: 'mermaid', title: 'The agent loop: plan, act, observe, repeat' },
  'family-tree':       { kind: 'mermaid', title: 'AI > Machine Learning > Deep Learning > Generative AI' },
  'neural-network':    { kind: 'mermaid', title: 'A neural network: input, hidden, and output layers' },
  'training-pipeline': { kind: 'mermaid', title: 'Training: pre-training, fine-tuning, then RLHF' },
  'transformer':       { kind: 'mermaid', title: 'Transformer: tokens to embeddings to attention to next-token probabilities' },
  'rag-pipeline':      { kind: 'mermaid', title: 'RAG: embed query, retrieve chunks, augment prompt, generate' },
  'mcp-architecture':  { kind: 'mermaid', title: 'MCP: a host app\'s client connects to tool/resource servers' },
  'tool-use':          { kind: 'mermaid', title: 'Tool use: user, model, tool round-trip' },
};
```

- [ ] **Step 3: Add `<figure>`+markers at each section.** For each id, find its section and insert the figure block (same pattern as Task 4 step 2). Target headings (grep to locate): family-tree → "AI / ML / DL" / "family tree"; neural-network → "Neural Networks"; training-pipeline → "RLHF" / "How … trained"; transformer → "Transformer" / LLM section; rag-pipeline → "How RAG Works"; mcp-architecture → "MCP" / Model Context Protocol; tool-use → "How Tool Use Works". Caption each with a one-line `.ann`.

- [ ] **Step 4: Build all and verify**

```bash
cd /Users/divakaran/arrcus_workspace/guides
node scripts/build-diagrams.mjs zero-to-hero
f=gen-ai-zero-to-hero-guide.html
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
echo "duplicate ids (expect none):"; grep -oE ' id="[^"]+"' "$f" | sort | uniq -d | head
echo "raw hex in any diagram (expect none): "; grep -oE '<!--DIAGRAM:[a-z-]+-->' "$f" | wc -l
```
Expected: `injected 8 diagram(s)`; div balanced; **no duplicate ids**; 8 markers present. Spot-check 2-3 in browser (dark/light).

- [ ] **Step 5: Commit**

```bash
git add diagrams/zero-to-hero/*.mmd diagrams/zero-to-hero/_registry.mjs gen-ai-zero-to-hero-guide.html
git commit -m "docs(zero-to-hero): add 7 diagrams — family tree, NN, training, transformer, RAG, MCP, tool use"
```

---

## Task 7: Author the 2 charts + timeline + 2 concepts (data files)

**Files:** Create `diagrams/zero-to-hero/{context-window-sizes.chart.json, pricing.chart.json, history-timeline.svg.json, embeddings-space.svg.json, token-fill.svg.json}`; update `_registry.mjs`; add markers; inject.

- [ ] **Step 1: Write the 5 data files** (values carry exact numbers per spec M4)

`context-window-sizes.chart.json`:
```json
{ "unit": "tokens", "bars": [
  { "label": "GPT-3.5 (2023)", "value": 4096, "display": "4K" },
  { "label": "GPT-4 / GPT-5 tier", "value": 128000, "display": "128K+" },
  { "label": "Claude (default)", "value": 200000, "display": "200K" },
  { "label": "Claude (extended)", "value": 1000000, "display": "1M" },
  { "label": "Gemini 3 Pro", "value": 1000000, "display": "1M" } ] }
```
`pricing.chart.json` (per-million output tokens; display carries in/out):
```json
{ "unit": "USD per 1M tokens", "bars": [
  { "label": "Claude Haiku 4.5", "value": 1.25, "display": "$0.25 / $1.25" },
  { "label": "Claude Sonnet 4.7", "value": 15, "display": "$3 / $15" },
  { "label": "Claude Opus 4.7", "value": 75, "display": "$15 / $75" },
  { "label": "GPT-4o (ref)", "value": 10, "display": "$2.50 / $10" } ] }
```
`history-timeline.svg.json`:
```json
{ "events": [
  { "year": "1950", "label": "Turing test" },
  { "year": "1956", "label": "\"AI\" coined" },
  { "year": "2012", "label": "Deep learning" },
  { "year": "2017", "label": "Transformers" },
  { "year": "2022", "label": "ChatGPT" },
  { "year": "2025+", "label": "Agents" } ] }
```
`embeddings-space.svg.json`:
```json
{ "clusters": [
  { "label": "animals", "color": "--blue",   "points": [[0.18,0.72],[0.24,0.66],[0.13,0.6]] },
  { "label": "royalty", "color": "--purple", "points": [[0.72,0.78],[0.8,0.7],[0.66,0.68]] },
  { "label": "food",    "color": "--orange", "points": [[0.5,0.2],[0.58,0.27],[0.44,0.16]] } ] }
```
`token-fill.svg.json`:
```json
{ "total": 200000, "segments": [
  { "label": "system prompt", "count": 4000,   "color": "--purple" },
  { "label": "history",       "count": 120000, "color": "--blue" },
  { "label": "your message",  "count": 6000,   "color": "--teal" },
  { "label": "free space",    "count": 70000,  "color": "--surface3" } ] }
```

- [ ] **Step 2: Add the 5 entries to `_registry.mjs`**

```javascript
  'context-window-sizes': { kind: 'barChart', data: 'context-window-sizes.chart.json', title: 'Context window sizes by model' },
  'pricing':              { kind: 'barChart', data: 'pricing.chart.json', title: 'Illustrative API pricing per 1M tokens' },
  'history-timeline':     { kind: 'timeline', data: 'history-timeline.svg.json', title: 'A brief history of AI' },
  'embeddings-space':     { kind: 'scatter',  data: 'embeddings-space.svg.json', title: 'Embeddings: similar meanings cluster together' },
  'token-fill':           { kind: 'tokenFill', data: 'token-fill.svg.json', title: 'A context window filling up' },
```

- [ ] **Step 3: Add `<figure>`+markers** at: context-window-sizes → the "Tokens & Context Windows" comparison; pricing → "Pricing & Cost Awareness"; history-timeline → "A Brief History of AI"; embeddings-space → "Why Embeddings Matter"; token-fill → "Tokens & Context Windows" (cost/fill point). Keep the source tables adjacent for the two charts (spec M4) — add the chart above/below the table, don't delete the table.

- [ ] **Step 4: Build all + verify (now 13)**

```bash
cd /Users/divakaran/arrcus_workspace/guides
node scripts/build-diagrams.mjs zero-to-hero
f=gen-ai-zero-to-hero-guide.html
echo "markers: $(grep -oE '<!--DIAGRAM:[a-z-]+-->' "$f" | wc -l) (expect 13)"
echo "dup ids:"; grep -oE ' id="[^"]+"' "$f" | sort | uniq -d | head
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
```
Expected: 13 markers, no dup ids, div balanced.

- [ ] **Step 5: Commit**

```bash
git add diagrams/zero-to-hero/*.json diagrams/zero-to-hero/_registry.mjs gen-ai-zero-to-hero-guide.html
git commit -m "docs(zero-to-hero): add 5 diagrams — context-window & pricing charts, timeline, embeddings, token-fill"
```

---

## Task 8: Full-guide integrity + idempotency check

**Files:** none (verify; fix-ups only).

- [ ] **Step 1: Idempotency — re-run build, expect zero diff**

```bash
cd /Users/divakaran/arrcus_workspace/guides
node scripts/build-diagrams.mjs zero-to-hero
git diff --stat gen-ai-zero-to-hero-guide.html
```
Expected: **no diff** (byte-stable on this machine). If there's churn, investigate nondeterministic ids in post-process before proceeding.

- [ ] **Step 2: Integrity sweep**

```bash
f=gen-ai-zero-to-hero-guide.html
o=$(grep -o "<div" "$f"|wc -l); c=$(grep -o "</div>" "$f"|wc -l); echo "div $o/$c"
echo "duplicate ids (expect none):"; grep -oE ' id="[^"]+"' "$f" | sort | uniq -d
echo "raw hex within any <svg> (expect none):"; grep -oE '#f0a00[0-9a-f]' "$f"
echo "svg count (expect 13):"; grep -oc '<svg' "$f"
grep -oE "go\('[a-z0-9-]+'\)" "$f"|sed "s/go('//;s/')//"|sort -u > /tmp/n.txt
grep -oE 'id="[a-z0-9-]+" class="section' "$f"|sed 's/id="//;s/" class.*//'|sort -u > /tmp/s.txt
echo "broken nav:"; comm -23 /tmp/n.txt /tmp/s.txt
```
Expected: divs balanced; no duplicate ids; no sentinel hex; 13 `<svg>`; no broken nav. Fix any issue, then re-run.

- [ ] **Step 3: Offline check** — open `gen-ai-zero-to-hero-guide.html` with the network disabled; all 13 diagrams render, no console errors.

- [ ] **Step 4: Commit** (only if fix-ups were made)

```bash
git add gen-ai-zero-to-hero-guide.html scripts/ diagrams/
git commit -m "fix(diagrams): integrity + idempotency fix-ups"
```

---

## Task 9: Visual review pass (dark + light)

**Files:** none (review; fix-ups only).

- [ ] **Step 1:** Open the guide; visit each of the 13 diagrams. Toggle dark/light on several.
- [ ] **Step 2: Check** — diagrams recolor with the theme (no fixed dark/light colors); arrowheads/markers render; text is legible; wide diagrams (training-pipeline, RAG, MCP) don't overflow (wrap in `<div class="diagram-scroll">` if needed); charts show value labels; no element looks off-palette.
- [ ] **Step 3:** Fix layout issues (overflow wrap, caption wording, a too-wide flowchart → switch `LR`↔`TB`), re-run the build for any changed `.mmd`, re-verify integrity, and commit `fix(diagrams): visual polish` if changed.

---

## Task 10: Super-review (R1)

**Files:** the guide (fix-ups), plus a review doc.

- [ ] **Step 1 (manual/assisted — invoke the skill interactively, not a shell command): Run the `super-review` skill** on `gen-ai-zero-to-hero-guide.html` focused on the diagrams: technical accuracy of each diagram vs the prose it sits in, theming correctness, accessibility (`<title>`/`<desc>` present and accurate), and that no diagram contradicts the chapter.
- [ ] **Step 2:** Apply confirmed fixes (edit the `.mmd`/data, re-run the build, re-verify), HIGH → MEDIUM → LOW.
- [ ] **Step 3: Final commit** — `docs(zero-to-hero): R1 diagram review fixes`. (Review doc `REVIEW-*.md` is gitignored.)

---

## Self-Review Notes (writer to writer)

- **Spec coverage:** pipeline (§3) → Tasks 2–3, 5; theming/CSS-vars (§3.3) → Tasks 2–4; npx-on-demand+pin (§2) → Task 1+conventions; id/marker uniquify (§3.2 H1) → Task 3 postprocess + Tasks 6–8 dup-id checks; recolor-not-drop `<style>` (R2-1) → postprocess step 3 (scope) + step 4 (recolor), never drops; charts keep numbers (M4) → Task 7 `display` fields + keep tables; no `--accent` (M5) → Task 1 step 3 + theme-map `--blue`; orphan-marker error (L1) → `inject()` throws / warns; byte-idempotency same-env (R2-2) → Task 8 step 1; templates emit prefixed/no ids (R2-5) → templates use no element ids (only shapes), satisfying the rule; inventory (§4, 13) → Tasks 3,6,7.
- **No placeholders:** every script/template/`.mmd`/data file is given in full; the only `<…>` is the version pin, resolved in Task 1 step 2 and propagated.
- **Naming consistency:** `REGISTRY` kinds (`mermaid`,`barChart`,`timeline`,`scatter`,`tokenFill`) match `templates.mjs` exports and the `build()` dispatch; marker id == registry key == `.mmd`/data basename == `d-<id>-` prefix.
- **Verification is domain-appropriate:** no pytest — "tests" are `lint: []` (no stray hex), no-duplicate-id grep, div-balance, byte-idempotent re-run, offline render, dark/light toggle.
- **One commit per task** (`feat(diagrams)` infra / `docs(zero-to-hero)` content / `fix(diagrams)` fix-ups).
