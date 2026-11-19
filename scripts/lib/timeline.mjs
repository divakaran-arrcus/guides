// Render pktd pipeline timelines ("T+ <ms> ms  <stage> begin|end" lines) as HTML bars.
// A capture may hold several runs (before / --- with --fixed: / after); all runs in one figure
// share one time scale, so a fix visibly shortens the bar. Styling lives in the guide's .tl CSS.

const esc = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
  .replace(/"/g, '&quot;').replace(/'/g, '&#39;');

const STAGES = ['hold', 'compute', 'rib', 'push'];

export function parseTimelines(text) {
  const runs = [];
  let cur = null, fixes = null;
  for (const line of String(text).split('\n')) {
    let m;
    if ((m = line.match(/^fixes applied: (\S+)/))) { fixes = m[1]; continue; }
    if ((m = line.match(/^pktd route pipeline .*variant=(\S+)/))) {
      cur = { label: fixes ? `fixed: ${fixes}` : m[1], stages: [], total: 0 };
      fixes = null;
      runs.push(cur);
      continue;
    }
    if (!cur) continue;
    if ((m = line.match(/^T\+\s*([\d.]+) ms\s+(\w+)\s+(begin|end)$/)) && STAGES.includes(m[2])) {
      const t = Number(m[1]);
      let s = cur.stages.find(x => x.name === m[2]);
      if (!s) { s = { name: m[2], begin: t, end: t }; cur.stages.push(s); }
      if (m[3] === 'begin') s.begin = t; else s.end = t;
    } else if ((m = line.match(/^converged_ms=([\d.]+)/))) {
      cur.total = Number(m[1]);
    }
  }
  return runs.filter(r => r.stages.length);
}

export function renderTimeline(text) {
  const runs = parseTimelines(text);
  if (!runs.length) return '<div class="tl"><div class="tl-cap">no timeline in this capture</div></div>\n';
  const scale = Math.max(...runs.map(r => r.total || Math.max(...r.stages.map(s => s.end))));
  const pct = v => (100 * v / scale).toFixed(2);
  const out = ['<div class="tl">'];
  for (const r of runs) {
    const done = r.total ? `converged in ${r.total.toFixed(1)} ms` : 'no converged_ms line in the capture';
    out.push(`<div class="tl-cap"><strong>${esc(r.label)}</strong> · ${done}</div>`);
    for (const s of r.stages) {
      const d = s.end - s.begin;
      out.push(`<div class="tl-row"><span class="tl-lab">${esc(s.name)}</span><span class="tl-track">`
        + `<span class="tl-bar tl-${esc(s.name)}" style="left:${pct(s.begin)}%;width:${pct(d)}%" title="${esc(s.name)}: ${d.toFixed(1)} ms"></span>`
        + `</span><span class="tl-ms">${d.toFixed(1)} ms</span></div>`);
    }
  }
  out.push(`<div class="tl-cap">One scale for every run in this figure: the full width is ${scale.toFixed(1)} ms.</div>`);
  out.push('</div>');
  return out.join('\n') + '\n';
}
