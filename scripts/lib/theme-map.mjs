// Sentinel hex -> guide CSS token. Zero-to-Hero has NO --accent; accent role = --blue.
export const THEME_MAP = {
  '#f0a001': 'var(--bg)',          // backgrounds / cluster / edge-label bg
  '#f0a002': 'var(--surface2)',    // node fill
  '#f0a003': 'var(--border)',      // borders
  '#f0a004': 'currentColor',       // text
  '#f0a005': 'var(--text-dim)',    // edges / signals
  '#f0a006': 'var(--blue)',        // accent role (Part 1)
  '#f0a007': 'var(--surface3)',    // tertiary fill
};
// Any sentinel family hex not mapped above (defensive): map to currentColor.
export const SENTINEL_RE = /#f0a00[0-9a-f]/gi;
