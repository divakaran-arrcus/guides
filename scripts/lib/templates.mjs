// All templates emit inline SVG using var(--token) only, with no element ids.
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
