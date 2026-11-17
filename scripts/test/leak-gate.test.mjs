import { test } from 'node:test';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const GATE = join(dirname(fileURLToPath(import.meta.url)), '..', 'leak-gate.sh');
const tmp = mkdtempSync(join(tmpdir(), 'lg-'));
const list = join(tmp, 'terms');
writeFileSync(list, 'zzleakzz\n');
const file = (name, body) => { const p = join(tmp, name); writeFileSync(p, body); return p; };
const run = (args, env = {}, cwd = tmp) => {
  const e = { ...process.env, ...env };
  if (!('LAB_HOST' in env)) delete e.LAB_HOST;
  return spawnSync('sh', [GATE, ...args], { env: e, cwd, encoding: 'utf8' });
};

test('clean file -> 0', () => {
  assert.equal(run([file('c.txt', 'nothing here\n')], { LEAK_TERMS: list }).status, 0);
});
test('term hit -> 1', () => {
  assert.equal(run([file('h.txt', 'has ZZLeakZZ inside\n')], { LEAK_TERMS: list }).status, 1);
});
test('missing list -> 2', () => {
  assert.equal(run([file('c.txt', 'x\n')], { LEAK_TERMS: '/nonexistent' }).status, 2);
});
test('missing file arg -> 2', () => {
  assert.equal(run([join(tmp, 'nope')], { LEAK_TERMS: list }).status, 2);
});
test('LAB_HOST user part is word-bounded: lab does not match labs', () => {
  const f = file('w.txt', 'docker run ubuntu:24.04 --hostname labs\n');
  assert.equal(run([f], { LEAK_TERMS: list, LAB_HOST: 'lab@10.0.0.9' }).status, 0);
});
test('LAB_HOST address hit -> 1', () => {
  const f = file('a.txt', 'ssh to 10.0.0.9 now\n');
  assert.equal(run([f], { LEAK_TERMS: list, LAB_HOST: 'lab@10.0.0.9' }).status, 1);
});
test('LAB_HOST unset: host check skipped', () => {
  assert.equal(run([file('e.txt', 'anything\n')], { LEAK_TERMS: list }).status, 0);
});
test('--staged with only a deletion -> 0', () => {
  const repo = mkdtempSync(join(tmpdir(), 'lgr-'));
  const g = (...a) => spawnSync('git', a, { cwd: repo, encoding: 'utf8' });
  g('init', '-q'); g('config', 'user.email', 't@t'); g('config', 'user.name', 't');
  writeFileSync(join(repo, 'f.txt'), 'hello\n');
  g('add', 'f.txt'); g('commit', '-qm', 'init');
  g('rm', '-q', 'f.txt');
  assert.equal(run(['--staged'], { LEAK_TERMS: list }, repo).status, 0);
  rmSync(repo, { recursive: true, force: true });
});
test('--staged with a leaking added line -> 1', () => {
  const repo = mkdtempSync(join(tmpdir(), 'lgr-'));
  const g = (...a) => spawnSync('git', a, { cwd: repo, encoding: 'utf8' });
  g('init', '-q');
  writeFileSync(join(repo, 'f.txt'), 'zzleakzz\n');
  g('add', 'f.txt');
  assert.equal(run(['--staged'], { LEAK_TERMS: list }, repo).status, 1);
  rmSync(repo, { recursive: true, force: true });
});

test('empty term list -> 2 (fail closed)', () => {
  const empty = join(tmp, 'empty-terms'); writeFileSync(empty, '');
  assert.equal(run([file('ec.txt', 'anything\n')], { LEAK_TERMS: empty }).status, 2);
});
test('whitespace-only term list -> 2 (fail closed)', () => {
  const ws = join(tmp, 'ws-terms'); writeFileSync(ws, '  \n\t\n\n');
  assert.equal(run([file('wc.txt', 'anything\n')], { LEAK_TERMS: ws }).status, 2);
});
