#!/usr/bin/env node
// Render diagrams/<guide>/*.mmd (+ *.json) -> themed inline SVG -> inject into the guide HTML.
import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, mkdtempSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, dirname } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { postprocess, lintSvg } from './lib/postprocess.mjs';

const PIN = '@mermaid-js/mermaid-cli@11.4.2';
const ROOT = '/Users/divakaran/arrcus_workspace/guides';
const HERE = dirname(fileURLToPath(import.meta.url));
const GUIDE = process.argv[2] || 'zero-to-hero';
const ONLY = process.argv[3];
const HTML = join(ROOT, `gen-ai-${GUIDE}-guide.html`);
const DIR = join(ROOT, 'diagrams', GUIDE);
const CHROME = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';

const { REGISTRY } = await import(pathToFileURL(join(DIR, '_registry.mjs')).href);

function renderMermaid(id) {
  const out = join(mkdtempSync(join(tmpdir(), 'dg-')), `${id}.svg`);
  execFileSync('npx', ['-y', PIN, '-p', join(ROOT, 'puppeteer-config.json'),
    '-c', join(ROOT, 'diagrams', '_theme.json'),
    '-i', join(DIR, `${id}.mmd`), '-o', out],
    { stdio: 'inherit', env: { ...process.env, PUPPETEER_EXECUTABLE_PATH: CHROME } });
  return readFileSync(out, 'utf8');
}

async function build(id, meta) {
  let svg;
  if (meta.kind === 'mermaid') {
    svg = postprocess(renderMermaid(id), id, meta.title, meta.desc);
  } else {
    const tpl = await import(pathToFileURL(join(HERE, 'lib', 'templates.mjs')).href);
    const data = JSON.parse(readFileSync(join(DIR, meta.data), 'utf8'));
    svg = tpl[meta.kind](id, data, meta.title);
  }
  const problems = lintSvg(svg);
  if (problems.length) throw new Error(`[${id}] ${problems.join('; ')}`);
  return svg;
}

function inject(html, id, svg) {
  const open = `<!--DIAGRAM:${id}-->`, close = `<!--/DIAGRAM:${id}-->`;
  const i = html.indexOf(open), j = html.indexOf(close);
  if (i === -1 || j === -1) throw new Error(`marker for "${id}" not found in HTML (add <figure>+markers first)`);
  return html.slice(0, i + open.length) + '\n' + svg + '\n' + html.slice(j);
}

let html = readFileSync(HTML, 'utf8');
const ids = ONLY ? [ONLY] : Object.keys(REGISTRY);
for (const id of ids) html = inject(html, id, await build(id, REGISTRY[id]));
for (const m of html.matchAll(/<!--DIAGRAM:([a-z0-9-]+)-->/g))
  if (!REGISTRY[m[1]]) console.warn(`WARN: marker "${m[1]}" has no registry entry`);
writeFileSync(HTML, html);
console.log(`injected ${ids.length} diagram(s) into ${HTML}`);
