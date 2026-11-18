import { existsSync } from 'node:fs';
import { join } from 'node:path';

// <root>/<slug>-guide.html if present, else the legacy <root>/gen-ai-<slug>-guide.html.
export function resolveGuideHtml(root, slug) {
  const plain = join(root, `${slug}-guide.html`);
  return existsSync(plain) ? plain : join(root, `gen-ai-${slug}-guide.html`);
}
