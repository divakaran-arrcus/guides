import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { parseTimelines, renderTimeline } from '../lib/timeline.mjs';
import { syncHtml } from '../sync-labs.mjs';

const sha = s => createHash('sha256').update(s).digest('hex');
const block = (variant, total, extra = '') => `${extra}pktd route pipeline  lab=x variant=${variant}  nodes=1 routes=1 changes=1
T+     0.000 ms  event      link change injected
T+     0.100 ms  hold       begin
T+    50.000 ms  hold       end
T+    50.000 ms  compute    begin
T+    ${(total - 10).toFixed(3)} ms  compute    end
T+    ${(total - 10).toFixed(3)} ms  rib        begin
T+    ${(total - 9).toFixed(3)} ms  rib        end
T+    ${(total - 9).toFixed(3)} ms  push       begin
T+    ${total.toFixed(3)} ms  push       end
converged_ms=${total.toFixed(3)}
`;

test('parseTimelines reads one block per run, with stage spans and the label', () => {
  const t = parseTimelines(block('before', 200) + '--- with --fixed:\n' + block('fixed', 100));
  assert.equal(t.length, 2);
  assert.equal(t[0].label, 'before');
  assert.equal(t[1].label, 'fixed');
  assert.equal(t[0].total, 200);
  assert.deepEqual(t[0].stages.find(s => s.name === 'hold'), { name: 'hold', begin: 0.1, end: 50 });
});

test('a boss run is labelled by its fixes', () => {
  const t = parseTimelines(block('fixed', 90, 'fixes applied: push,hot\n'));
  assert.equal(t[0].label, 'fixed: push,hot');
});

test('blocks share one time scale', () => {
  const html = renderTimeline(block('before', 200) + block('fixed', 100));
  const widths = [...html.matchAll(/tl-hold" style="left:([\d.]+)%;width:([\d.]+)%/g)].map(m => Number(m[2]));
  assert.equal(widths.length, 2);
  assert.ok(Math.abs(widths[0] - widths[1]) < 0.01, 'same hold length → same width');
  assert.match(html, /converged in 200\.0 ms/);
  assert.match(html, /converged in 100\.0 ms/);
});

test('a run without converged_ms says so instead of claiming 0 ms', () => {
  const html = renderTimeline(block('before', 80).replace(/^converged_ms=.*$/m, ''));
  assert.match(html, /no converged_ms line/);
  assert.doesNotMatch(html, /converged in 0\.0 ms/);
});

test('no timeline in the text renders an explicit note, not an empty figure', () => {
  assert.match(renderTimeline('nothing here\n'), /no timeline/);
});

test('sync-labs TL round-trips and flags stale captures', () => {
  const dir = mkdtempSync(join(tmpdir(), 'tl-'));
  mkdirSync(join(dir, 'captures'));
  writeFileSync(join(dir, 'a.c'), 'int a;\n');
  writeFileSync(join(dir, 'captures', 'a.timeline.txt'), `# src a.c ${sha('int a;\n')}\n# cmd x\n${block('before', 120)}`);
  const page = '<!--TL:a.timeline--><!--/TL:a.timeline-->';
  const w = syncHtml(page, dir, 'write');
  assert.deepEqual(w.errors, []);
  assert.match(w.html, /class="tl"/);
  assert.deepEqual(syncHtml(w.html, dir, 'check').errors, []);
  writeFileSync(join(dir, 'a.c'), 'int b;\n');
  assert.match(syncHtml(w.html, dir, 'check').errors[0], /TL a\.timeline: stale capture/);
});
