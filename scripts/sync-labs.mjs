#!/usr/bin/env node
// Fill <!--SRC:path-->…<!--/SRC:path--> and <!--CAP:id-->…<!--/CAP:id--> blocks in a guide
// from labs/<slug>/ files and captures/<id>.txt. --write fills them; --check only verifies.
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { resolveGuideHtml } from './lib/guide-path.mjs';

export const escapeHtml = s => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
  .replace(/"/g, '&quot;').replace(/'/g, '&#39;');
export const unescapeHtml = s => s.replace(/&#39;/g, "'").replace(/&quot;/g, '"')
  .replace(/&gt;/g, '>').replace(/&lt;/g, '<').replace(/&amp;/g, '&');

// spec "1-2,4" -> those 1-based lines; gaps between ranges become one "⋯" line.
export function selectLines(text, spec) {
  if (spec === undefined) return text;
  const lines = text.split('\n');
  if (lines[lines.length - 1] === '') lines.pop();
  const out = [];
  let prevEnd = null;
  for (const part of spec.split(',')) {
    const [a, b = a] = part.split('-').map(Number);
    if (prevEnd !== null && a > prevEnd + 1) out.push('⋯');
    for (let n = a; n <= b && n <= lines.length; n++) out.push(lines[n - 1]);
    prevEnd = b;
  }
  return out.join('\n');
}

const sha256 = buf => createHash('sha256').update(buf).digest('hex');
const OPEN = /<!--(?:(SRC):([\w.\-/]+)|(CAP):([\w.\-]+))(?: lines=([\d,\-]+))?-->/g;

function expectedSrc(path, spec, labsDir) {
  if (path.startsWith('/') || path.split('/').includes('..')) return { error: `SRC ${path}: unsafe path (absolute or contains ..)` };
  const f = join(labsDir, path);
  if (!existsSync(f)) return { error: `SRC ${path}: file not found in ${labsDir}` };
  return { body: escapeHtml(selectLines(readFileSync(f, 'utf8'), spec)) };
}

function expectedCap(id, spec, labsDir) {
  const f = join(labsDir, 'captures', `${id}.txt`);
  if (!existsSync(f)) return { error: `CAP ${id}: captures/${id}.txt not found` };
  const lines = readFileSync(f, 'utf8').split('\n');
  let i = 0;
  const errs = [];
  for (; i < lines.length && /^# (src|cmd) /.test(lines[i]); i++) {
    const m = lines[i].match(/^# src (\S+) ([0-9a-f]{64})\s*$/);
    if (!m) continue;
    const sf = join(labsDir, m[1]);
    if (!existsSync(sf)) errs.push(`CAP ${id}: source ${m[1]} missing`);
    else if (sha256(readFileSync(sf)) !== m[2]) errs.push(`CAP ${id}: stale capture, ${m[1]} changed since capture`);
  }
  if (errs.length) return { error: errs.join('; ') };
  return { body: escapeHtml(selectLines(lines.slice(i).join('\n'), spec)) };
}

export function syncHtml(html, labsDir, mode) {
  const errors = [];
  let out = '';
  let pos = 0;
  OPEN.lastIndex = 0;
  for (;;) {
    OPEN.lastIndex = pos;
    const m = OPEN.exec(html);
    if (!m) break;
    const open = m[0], kind = m[1] ?? m[3], id = m[2] ?? m[4], spec = m[5];
    const bodyStart = m.index + open.length;
    const close = `<!--/${kind}:${id}-->`;
    const j = html.indexOf(close, bodyStart);
    if (j === -1) {
      errors.push(`${kind} ${id}: no closing marker`);
      out += html.slice(pos, bodyStart);
      pos = bodyStart;
      continue;
    }
    const exp = (kind === 'SRC' ? expectedSrc : expectedCap)(id, spec, labsDir);
    out += html.slice(pos, bodyStart);
    const current = html.slice(bodyStart, j);
    if (exp.error) { errors.push(exp.error); out += current; }
    else if (mode === 'write') out += exp.body;
    else {
      if (current !== exp.body) errors.push(`${kind} ${id}${spec ? ` lines=${spec}` : ''}: embedded block differs from source`);
      out += current;
    }
    pos = j;
  }
  out += html.slice(pos);
  return { html: mode === 'write' ? out : html, errors };
}

function main() {
  const args = process.argv.slice(2);
  const slug = args.find(a => !a.startsWith('--') && args[args.indexOf(a) - 1] !== '--labs');
  if (args.includes('--write') && args.includes('--check')) {
    console.error('sync-labs: --write and --check are mutually exclusive');
    process.exit(2);
  }
  const mode = args.includes('--write') ? 'write' : args.includes('--check') ? 'check' : null;
  const li = args.indexOf('--labs');
  if (!slug || !mode || (li !== -1 && !args[li + 1])) {
    console.error('usage: sync-labs.mjs <slug> --write|--check [--labs <dir>]');
    process.exit(2);
  }
  const root = join(dirname(fileURLToPath(import.meta.url)), '..');
  const labs = resolve(li !== -1 ? args[li + 1] : join(root, 'labs', slug));
  const file = resolveGuideHtml(root, slug);
  const { html, errors } = syncHtml(readFileSync(file, 'utf8'), labs, mode);
  if (errors.length) {
    for (const e of errors) console.error(`sync-labs: ${e}`);
    console.error(mode === 'write' ? 'sync-labs: errors; nothing written' : 'sync-labs: check failed');
    process.exit(1);
  }
  if (mode === 'write') { writeFileSync(file, html); console.log(`synced ${file}`); }
  else console.log(`sync-labs: ${file} in sync`);
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
