import { test } from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { escapeHtml, unescapeHtml, selectLines, syncHtml } from '../sync-labs.mjs';
import { resolveGuideHtml } from '../lib/guide-path.mjs';

const sha = s => createHash('sha256').update(s).digest('hex');
function lab(files = {}, caps = {}) {
  const dir = mkdtempSync(join(tmpdir(), 'sl-'));
  mkdirSync(join(dir, 'captures'), { recursive: true });
  for (const [f, c] of Object.entries(files)) {
    mkdirSync(join(dir, f, '..'), { recursive: true });
    writeFileSync(join(dir, f), c);
  }
  for (const [id, c] of Object.entries(caps)) writeFileSync(join(dir, 'captures', `${id}.txt`), c);
  return dir;
}
const src = (p, body = '', spec = '') => `<!--SRC:${p}${spec ? ` lines=${spec}` : ''}-->${body}<!--/SRC:${p}-->`;
const cap = (id, body = '', spec = '') => `<!--CAP:${id}${spec ? ` lines=${spec}` : ''}-->${body}<!--/CAP:${id}-->`;
const inner = (html, tag, id) => html.match(new RegExp(`<!--${tag}:${id}(?: lines=[\\d,-]+)?-->([\\s\\S]*?)<!--/${tag}:${id}-->`))[1];

test('escapeHtml round-trips C source', () => {
  const s = 'if (a < b && p->x > 0) puts("ok \'q\'");\n';
  assert.equal(unescapeHtml(escapeHtml(s)), s);
  assert.ok(!escapeHtml(s).includes('<'));
});

test('selectLines', () => {
  assert.equal(selectLines('a\nb\nc\nd\ne\n', '1-2,4'), 'a\nb\n⋯\nd');
  assert.equal(selectLines('a\nb\n', undefined), 'a\nb\n');
});

test('CAP strips only leading # src/# cmd headers', () => {
  const body = '# comment in body\nline2\n';
  const dir = lab({ 'x.c': 'int x;\n' }, { c1: `# src x.c ${sha('int x;\n')}\n# cmd ./x\n${body}` });
  const { html, errors } = syncHtml(cap('c1'), dir, 'write');
  assert.deepEqual(errors, []);
  assert.equal(unescapeHtml(inner(html, 'CAP', 'c1')), body);
});

test('SRC keeps # lines', () => {
  const mk = '# comment\nall:\n\tcc x.c\n# tail\n';
  const dir = lab({ Makefile: mk });
  const { html, errors } = syncHtml(src('Makefile'), dir, 'write');
  assert.deepEqual(errors, []);
  assert.equal(unescapeHtml(inner(html, 'SRC', 'Makefile')), mk);
});

test('write fills SRC and CAP; CAP excludes the # src header', () => {
  const c = 'int a<b;\n';
  const dir = lab({ 'a.c': c }, { 'a.run': `# src a.c ${sha(c)}\n# cmd ./a\nout & more\n` });
  const { html, errors } = syncHtml(`${src('a.c')}\n${cap('a.run')}`, dir, 'write');
  assert.deepEqual(errors, []);
  assert.equal(inner(html, 'SRC', 'a.c'), 'int a&lt;b;\n');
  assert.equal(inner(html, 'CAP', 'a.run'), 'out &amp; more\n');
  assert.ok(!html.includes('# src'));
});

test('SRC lines= excerpt is filled and checked', () => {
  const dir = lab({ 'a.c': 'l1\nl2\nl3\nl4\n' });
  const page = src('a.c', '', '2-3');
  const w = syncHtml(page, dir, 'write');
  assert.equal(inner(w.html, 'SRC', 'a.c'), 'l2\nl3');
  assert.deepEqual(syncHtml(w.html, dir, 'check').errors, []);
  writeFileSync(join(dir, 'a.c'), 'l1\nX\nl3\nl4\n');
  assert.equal(syncHtml(w.html, dir, 'check').errors.length, 1);
});

test('repeated id in two places: both filled and independent', () => {
  const dir = lab({ 'a.c': 'one\ntwo\n' });
  const page = `${src('a.c', '', '1-1')}\nmid\n${src('a.c')}`;
  const { html, errors } = syncHtml(page, dir, 'write');
  assert.deepEqual(errors, []);
  const bodies = [...html.matchAll(/<!--SRC:a\.c(?: lines=[\d,-]+)?-->([\s\S]*?)<!--\/SRC:a\.c-->/g)].map(m => m[1]);
  assert.deepEqual(bodies, ['one', 'one\ntwo\n']);
});

test('check flags drifted SRC naming the file', () => {
  const dir = lab({ 'a.c': 'v1\n' });
  const { html } = syncHtml(src('a.c'), dir, 'write');
  writeFileSync(join(dir, 'a.c'), 'v2\n');
  const { errors } = syncHtml(html, dir, 'check');
  assert.equal(errors.length, 1);
  assert.match(errors[0], /a\.c/);
});

test('check flags stale capture naming the capture id', () => {
  const dir = lab({ 'x.c': 'new\n' }, { c9: `# src x.c ${sha('old\n')}\n# cmd ./x\nout\n` });
  const { errors } = syncHtml(cap('c9', 'out\n'), dir, 'check');
  assert.equal(errors.length, 1);
  assert.match(errors[0], /c9/);
});

test('missing file is an error in both modes', () => {
  const dir = lab();
  for (const mode of ['write', 'check']) {
    assert.match(syncHtml(src('nope.c'), dir, mode).errors.join('\n'), /nope\.c/);
    assert.match(syncHtml(cap('nocap'), dir, mode).errors.join('\n'), /nocap/);
  }
});

test('write is idempotent', () => {
  const dir = lab({ 'a.c': 'x\n' }, { c: `# src a.c ${sha('x\n')}\n# cmd r\nhi\n` });
  const page = `${src('a.c')}${cap('c', '', '1-1')}`;
  const a = syncHtml(page, dir, 'write').html;
  const b = syncHtml(a, dir, 'write').html;
  assert.equal(a, b);
  assert.deepEqual(syncHtml(a, dir, 'check').errors, []);
});

test('check mode never modifies html', () => {
  const dir = lab({ 'a.c': 'x\n' });
  const page = src('a.c', 'stale');
  const r = syncHtml(page, dir, 'check');
  assert.equal(r.html, page);
  assert.equal(r.errors.length, 1);
});

test('resolveGuideHtml prefers <slug>-guide.html, falls back to gen-ai', () => {
  const root = mkdtempSync(join(tmpdir(), 'gp-'));
  writeFileSync(join(root, 'gen-ai-foo-guide.html'), '');
  assert.equal(resolveGuideHtml(root, 'foo'), join(root, 'gen-ai-foo-guide.html'));
  writeFileSync(join(root, 'foo-guide.html'), '');
  assert.equal(resolveGuideHtml(root, 'foo'), join(root, 'foo-guide.html'));
});

test('CAP id charset excludes "/": CAP:a/b is not a CAP block', () => {
  const dir = lab();
  const page = '<!--CAP:a/b-->keep<!--/CAP:a/b-->';
  const r = syncHtml(page, dir, 'write');
  assert.deepEqual(r.errors, []);
  assert.equal(r.html, page);
});
