import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, mkdirSync, writeFileSync, readFileSync, copyFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const SCRIPTS = join(dirname(fileURLToPath(import.meta.url)), '..');
function root() {
  const r = mkdtempSync(join(tmpdir(), 'ag-'));
  mkdirSync(join(r, 'scripts', 'lib'), { recursive: true });
  copyFileSync(join(SCRIPTS, 'assemble-guide.mjs'), join(r, 'scripts', 'assemble-guide.mjs'));
  copyFileSync(join(SCRIPTS, 'lib', 'guide-path.mjs'), join(r, 'scripts', 'lib', 'guide-path.mjs'));
  mkdirSync(join(r, '.build', 'q'), { recursive: true });
  writeFileSync(join(r, '.build', 'q', 'a.html'), 'NEW\n');
  writeFileSync(join(r, 'q-guide.html'), '<!--PART:a-->\nold\n<!--/PART:a-->\n');
  return r;
}
const run = (r, ...a) => spawnSync('node', [join(r, 'scripts', 'assemble-guide.mjs'), ...a], { encoding: 'utf8' });

test('in place: <slug> only', () => {
  const r = root();
  assert.equal(run(r, 'q').status, 0);
  assert.match(readFileSync(join(r, 'q-guide.html'), 'utf8'), /NEW/);
});
for (const order of ['slug-first', 'out-first']) {
  test(`--out leaves original untouched (${order})`, () => {
    const r = root(), o = join(r, 'o.html');
    const args = order === 'slug-first' ? ['q', '--out', o] : ['--out', o, 'q'];
    assert.equal(run(r, ...args).status, 0);
    assert.match(readFileSync(o, 'utf8'), /NEW/);
    assert.match(readFileSync(join(r, 'q-guide.html'), 'utf8'), /old/);
  });
}
test('no slug -> usage, exit 2', () => {
  assert.equal(run(root()).status, 2);
});
