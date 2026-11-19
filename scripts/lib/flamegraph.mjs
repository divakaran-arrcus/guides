// Render folded stacks ("outer;inner;leaf <count>" per line) as a static, themed flame graph SVG.
// Classic layout: root at the bottom, callees stacked above, siblings in alphabetical order,
// width proportional to samples. Colours come from CSS variables --flame-0..4 so the page's
// light/dark theme applies. Output is deterministic for a given input.

const esc = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
  .replace(/"/g, '&quot;').replace(/'/g, '&#39;');

const node = name => ({ name, value: 0, children: new Map() });

export function parseFolded(text) {
  const root = node('all');
  for (const line of String(text).split('\n')) {
    const m = line.match(/^(.*\S)\s+(\d+)\s*$/);
    if (!m) continue;
    const n = Number(m[2]);
    if (!n) continue;
    root.value += n;
    let cur = root;
    for (const f of m[1].split(';')) {
      if (!cur.children.has(f)) cur.children.set(f, node(f));
      cur = cur.children.get(f);
      cur.value += n;
    }
  }
  return root;
}

function hue(name) {
  let h = 0;
  for (const ch of name) h = (h * 31 + ch.codePointAt(0)) >>> 0;
  return h % 5;
}

// Rows actually drawn: frames under minPct are not drawn, so they must not add height either.
function depth(n, min) {
  let d = 0;
  for (const c of n.children.values()) if (c.value >= min) d = Math.max(d, depth(c, min));
  return d + 1;
}

// unit: 'samples' (on-CPU profiles) or 'us' (off-CPU graphs weighted by microseconds blocked).
export function renderFlame(text, { id, title, width = 960, rowH = 17, minPct = 0.5, unit = 'samples' } = {}) {
  const units = unit === 'us' ? 'µs' : 'samples';
  const root = parseFolded(text);
  const label = esc(title ?? `Flame graph ${id ?? ''}`.trim());
  const font = 'font-family="ui-monospace,SFMono-Regular,Menlo,monospace" font-size="11"';
  if (root.value === 0) {
    return `<svg class="flame" role="img" aria-label="${label}: no samples" viewBox="0 0 ${width} 40" width="100%">`
      + `<rect x="0.5" y="0.5" width="${width - 1}" height="39" rx="4" style="fill:none;stroke:var(--border)"/>`
      + `<text x="${width / 2}" y="24" text-anchor="middle" ${font} style="fill:var(--text-muted)">${unit === 'us' ? 'no blocked time recorded' : 'no samples: the process was not on a CPU'}</text></svg>\n`;
  }
  const total = root.value;
  const rows = depth(root, (minPct / 100) * total);
  const height = rows * rowH + 2;
  const out = [];
  const charW = 6.6;
  const draw = (n, x, level) => {
    const pct = (n.value / total) * 100;
    if (pct < minPct) return;
    const w = (n.value / total) * width;
    const y = height - (level + 1) * rowH;
    const tip = `${esc(n.name)} (${n.value} ${units}, ${pct.toFixed(1)}%)`;
    const fits = Math.floor((w - 6) / charW);
    let txt = '';
    if (fits >= 3) txt = n.name.length <= fits ? n.name : `${n.name.slice(0, fits - 2)}..`;
    out.push(`<g><title>${tip}</title><rect x="${x.toFixed(2)}" y="${y}" width="${Math.max(w - 0.5, 0.5).toFixed(2)}" height="${rowH - 1}" rx="2" style="fill:var(--flame-${hue(n.name)})"/>`
      + (txt ? `<text x="${(x + 3).toFixed(2)}" y="${y + rowH - 5}" ${font} style="fill:var(--flame-text)">${esc(txt)}</text>` : '')
      + '</g>');
    let cx = x;
    for (const c of [...n.children.values()].sort((a, b) => (a.name < b.name ? -1 : a.name > b.name ? 1 : 0))) {
      draw(c, cx, level + 1);
      cx += (c.value / total) * width;
    }
  };
  draw(root, 0, 0);
  return `<svg class="flame" role="img" aria-label="${label}" viewBox="0 0 ${width} ${height}" width="100%">\n${out.join('\n')}\n</svg>\n`;
}
