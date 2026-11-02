#!/usr/bin/env node
// Splice .build/<slug>/<part>.html fragments into gen-ai-<slug>-guide.html between <!--PART:x--> markers.
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const SLUG = process.argv[2];
if (!SLUG) { console.error('usage: assemble-guide.mjs <slug>'); process.exit(2); }
const HTML = join(ROOT, `gen-ai-${SLUG}-guide.html`);
const DIR = join(ROOT, '.build', SLUG);

let html = readFileSync(HTML, 'utf8');
const opens = [...html.matchAll(/<!--PART:([a-z0-9-]+)-->/g)].map(m => m[1]);
// Refuse before writing anything: a missing or empty fragment would blank that Part in place.
const missing = opens.filter(id => {
  const frag = join(DIR, `${id}.html`);
  return !existsSync(frag) || !readFileSync(frag, 'utf8').trim();
});
if (missing.length) {
  console.error(`missing or empty fragment(s) in ${DIR}: ${missing.join(', ')} — nothing written`);
  process.exit(1);
}
for (const id of opens) {
  const open = `<!--PART:${id}-->`, close = `<!--/PART:${id}-->`;
  const i = html.indexOf(open), j = html.indexOf(close);
  if (j === -1 || j < i) { console.error(`unpaired marker: ${id}`); process.exit(1); }
  const body = readFileSync(join(DIR, `${id}.html`), 'utf8').trim();
  html = html.slice(0, i + open.length) + '\n' + body + '\n' + html.slice(j);
}
writeFileSync(HTML, html);
console.log(`assembled ${opens.length} fragment(s) into ${HTML}`);
