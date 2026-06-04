import { THEME_MAP, SENTINEL_RE } from './theme-map.mjs';

// mmdc 11.4.2 hardcodes a few colors in its default CSS that are NOT driven by
// themeVariables (so never become sentinels). Map them to tokens so nothing
// raw survives. .arrowheadPath blue is rendered (it's the edge arrowheads) and
// must track the edge/line color; the black KaTeX/state-start rules are for
// unused diagram features but still ship in the inlined <style>.
// sequenceDiagram also emits its own hardcoded greys (#eaeaea, #666, #999) and
// a slightly different arrowhead blue (#0f5ffa) that are not theme-driven.
const DEFAULT_COLOR_MAP = {
  '#0f5ffe': 'var(--text-dim)', // .arrowheadPath -> same token as lineColor (#f0a005)
  '#0f5ffa': 'var(--text-dim)', // sequenceDiagram arrowhead variant
  '#eaeaea': 'var(--border)',   // sequenceDiagram actor/loop box stroke
  '#666':    'var(--text-muted)', // sequenceDiagram label text
  '#999':    'var(--text-muted)', // sequenceDiagram loop/alt text
  '#000000': 'currentColor',    // .node circle .state-start (unused here)
  '#000': 'currentColor',       // .node .katex path (unused here)
};
const DEFAULT_COLOR_RE = /#0f5ffa|#0f5ffe|#eaeaea|#666(?![0-9a-fA-F])|#999(?![0-9a-fA-F])|#000000|#000(?![0-9a-fA-F])/gi;

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

  // 4) Recolor: every sentinel hex -> token. (Assert none remain via lintSvg.)
  svg = svg.replace(SENTINEL_RE, (hex) => THEME_MAP[hex.toLowerCase()] || 'currentColor');
  // 4b) Map mmdc's hardcoded non-sentinel default colors to tokens too.
  svg = svg.replace(DEFAULT_COLOR_RE, (hex) => DEFAULT_COLOR_MAP[hex.toLowerCase()] || 'currentColor');

  // 5) Responsive + a11y on the root <svg>: drop fixed width/height/style, keep viewBox.
  svg = svg.replace(/<svg([^>]*?)>/, (m, attrs) => {
    let a = attrs
      .replace(/\swidth="[^"]*"/, '')
      .replace(/\sheight="[^"]*"/, '')
      .replace(/\sstyle="[^"]*"/, '')
      .replace(/\srole="[^"]*"/, '')                 // drop mmdc's role to avoid a duplicate
      .replace(/\saria-roledescription="[^"]*"/, ''); // pairs with the dropped graphics role
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
