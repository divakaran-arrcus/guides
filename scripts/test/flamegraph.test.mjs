import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { parseFolded, renderFlame } from '../lib/flamegraph.mjs';
import { syncHtml } from '../sync-labs.mjs';

const sha = s => createHash('sha256').update(s).digest('hex');

test('parseFolded sums counts into a tree', () => {
  const root = parseFolded('a;b 3\na;c 1\n');
  assert.equal(root.name, 'all');
  assert.equal(root.value, 4);
  assert.equal(root.children.get('a').value, 4);
  assert.equal(root.children.get('a').children.get('b').value, 3);
});

test('parseFolded ignores blank and malformed lines, keeps spaces in frames', () => {
  const root = parseFolded('\nnot a count\nmain;do work 2\n');
  assert.equal(root.value, 2);
  assert.ok(root.children.get('main').children.has('do work'));
});

test('empty profile renders a labelled no-samples svg', () => {
  const svg = renderFlame('', { id: 'e' });
  assert.match(svg, /no samples/);
  assert.equal(svg.match(/<svg/g).length, 1);
  assert.ok(svg.trimEnd().endsWith('</svg>'));
});

test('frame names are escaped', () => {
  const svg = renderFlame("main;foo<int>&'x' 5\n", { id: 'x' });
  assert.ok(svg.includes('foo&lt;int&gt;&amp;&#39;x&#39;'));
  assert.ok(!svg.includes('foo<int>'));
});

test('tiny frames are not drawn but still count toward the total', () => {
  const svg = renderFlame('main;big 999\nmain;tiny 1\n', { id: 't' });
  assert.ok(svg.includes('big'));
  assert.ok(!svg.includes('tiny'));
  assert.match(svg, /all \(1000 samples, 100(\.0)?%\)/);
});

test('output is deterministic and themed by CSS variables', () => {
  const f = 'main;a;b 3\nmain;c 2\n';
  assert.equal(renderFlame(f, { id: 'd' }), renderFlame(f, { id: 'd' }));
  assert.match(renderFlame(f, { id: 'd' }), /var\(--flame-[0-4]\)/);
});

test('sync-labs FLAME round-trips and flags stale captures', () => {
  const dir = mkdtempSync(join(tmpdir(), 'fl-'));
  mkdirSync(join(dir, 'captures'));
  writeFileSync(join(dir, 'a.c'), 'int a;\n');
  writeFileSync(join(dir, 'captures', 'a.folded.txt'), `# src a.c ${sha('int a;\n')}\n# cmd x\nmain;work 7\n`);
  const page = '<!--FLAME:a.folded--><!--/FLAME:a.folded-->';
  const w = syncHtml(page, dir, 'write');
  assert.deepEqual(w.errors, []);
  assert.match(w.html, /<svg[\s\S]*work[\s\S]*<\/svg>/);
  assert.deepEqual(syncHtml(w.html, dir, 'check').errors, []);
  writeFileSync(join(dir, 'a.c'), 'int b;\n');
  const c = syncHtml(w.html, dir, 'check');
  assert.equal(c.errors.length, 1);
  assert.match(c.errors[0], /FLAME a\.folded: stale capture/);
});
