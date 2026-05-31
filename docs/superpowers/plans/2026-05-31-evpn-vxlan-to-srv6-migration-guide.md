# EVPN-VXLAN to EVPN-SRv6 Migration Guide Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. The `guide-builder` skill should also be loaded — it has the canonical component vocabulary for this repo's HTML guides.

**Goal:** Build a single-page HTML guide titled "EVPN-VXLAN to EVPN-SRv6 Migration Guide" — 25 flat h2 sections covering the comparison (Part 1 + Part 2) and the migration playbook (Part 3) plus operational (Part 4) + reference. All body prose in point-wise format from the start.

**Architecture:** One self-contained `evpn-vxlan-to-srv6-migration-guide.html` file. Sidebar-driven SPA pattern (sections with `display: none`, JS `go(id, event)` activates one — matches the networking-guide template). Reuses the existing networking-guide chrome (fonts, color tokens, packet diagrams, comparison grids, info boxes, step flows, flv-grids); ports the `.ph` mini-heading class from `gen-ai-hero-to-architect-guide.html` for point-wise body prose throughout.

**Tech Stack:** Vanilla HTML + CSS + JS. No build step. Google Fonts (DM Sans, Source Serif 4, JetBrains Mono).

**Reference inputs:**
- Spec: `/Users/divakaran/arrcus_workspace/guides/docs/superpowers/specs/2026-05-31-evpn-vxlan-to-srv6-migration-design.md`
- Template (chrome donor): `/Users/divakaran/arrcus_workspace/guides/mpls-to-srv6-migration-guide.html` (closest playbook-structure peer)
- CSS donor for `.ph`: `/Users/divakaran/arrcus_workspace/guides/gen-ai-hero-to-architect-guide.html` lines 161–164
- Sibling guides for prereq cross-links (not modified): `bgp-evpn-guide.html`, `dc-fabric-design-guide.html`, `srv6-complete-guide.html`, `srv6-network-programming-guide.html`

**Guide-section numbering note:** Throughout this plan, "§N" refers to the **guide's** h2 sections per the spec (§FM-1, §FM-2, §1–§23). Not spec-doc sections.

---

## File Structure

Single artifact:
- Create: `/Users/divakaran/arrcus_workspace/guides/evpn-vxlan-to-srv6-migration-guide.html`
- Modify: `/Users/divakaran/arrcus_workspace/guides/README.md` (add a row to the "SRv6 Advanced & Migration" bucket — final task)
- Modify: `/Users/divakaran/arrcus_workspace/guides/IDEAS.md` (mark #6 Done — final task)

No other files. Embedded CSS + JS in the HTML file (matches the rest of the repo's guides).

---

## Conventions used in this plan

**Each "section task" follows the same pattern:**
1. Add the `<div id="…" class="section">` scaffold with `<div class="sec-title">` + `<p class="sec-sub">` lede
2. Add point-wise body inside a `<div class="card">` wrapper — short prose lede sentence, then `<h4 class="ph">` mini-headings with `<ul>/<li>` bullet lists for each idea
3. Add the section's visual components (`.pkt` / `.cmp` / `.stp` / `.flv-grid` / `.tbl` / `.info`) per the spec's §7 brief
4. Verify structurally (grep for the new section id; tag balance)
5. Commit with message `docs(evpn-srv6): write §N — <title>`

**Content brief per section (used inside steps):** the spec's §7 brief lists what the section covers, the mini-heading concepts, and which visual components to include. The brief lists *what* to write; the writer produces the prose. Briefs are quoted verbatim from the spec where applicable; no paraphrasing required.

**Reference topology — used in every walk and migration scenario:**
- 2-spine / 4-leaf fabric (Spine-1, Spine-2; Leaf-1 … Leaf-4)
- Host-A on Leaf-1, Host-B on Leaf-3 (same VRF/VNI for L2 walks)
- Tenants: VLAN-100 → VNI-10100, VLAN-200 → VNI-10200, one L3 VRF spans both
- States: State A (pure VXLAN, "before"), State B (pure SRv6 with uSID, "after"), State M (mid-migration, dual-encap with gateway at Leaf-2/4 = border-leaf default)
- uN locator block: `fc00:0:1::/48`, loopbacks `203.0.113.{1-4}` (RFC 5737 docs prefix), IPv6 loopbacks `2001:db8:1::{1-4}/128`
- Service SIDs: `fc00:0:1:1:e000::` (uDT2U for VNI-10100), `fc00:0:1:1:e001::` (uDT2U for VNI-10200), `fc00:0:1:1:e100::` (uDT4 for the L3 VRF)

These identifiers must be reused verbatim across §3, §6, §7, §8, §15, §16, §17.

**Commit message convention:** scaffold/structural tasks use `feat(evpn-srv6): …`; section/content tasks use `docs(evpn-srv6): …`; fix-up commits use `fix(evpn-srv6): …`.

---

## Task 1: Scaffold the HTML file from the networking-guide template

**Files:**
- Create: `/Users/divakaran/arrcus_workspace/guides/evpn-vxlan-to-srv6-migration-guide.html`
- Read: `/Users/divakaran/arrcus_workspace/guides/mpls-to-srv6-migration-guide.html` (template — closest playbook-structure peer)
- Read: `/Users/divakaran/arrcus_workspace/guides/gen-ai-hero-to-architect-guide.html` lines 161–164 (`.ph` CSS to port)

- [ ] **Step 1: Copy the template as starting point**

```bash
cp /Users/divakaran/arrcus_workspace/guides/mpls-to-srv6-migration-guide.html \
   /Users/divakaran/arrcus_workspace/guides/evpn-vxlan-to-srv6-migration-guide.html
```

- [ ] **Step 2: Update `<title>` and sidebar brand**

Edit the new file:
- `<title>EVPN-VXLAN to EVPN-SRv6 Migration Guide</title>`
- Sidebar brand `<h1>` → `EVPN-VXLAN → EVPN-SRv6`
- Sidebar brand tagline `<p>` → `Migration guide — comparison + playbook`

- [ ] **Step 3: Port the `.ph` mini-heading CSS from gen-ai-hero-to-architect-guide.html**

Find the existing `.card h4 {` rule in the file's `<style>` block (the networking-guide template defines `.card h4`; the new `.card h4.ph` rule must follow it). Append immediately after the existing `.card h4` rule:

```css
  .card h4.ph { font-size: 13.5px; font-weight: 700; color: var(--text); margin: 16px 0 6px; }
  .card h4.ph::before { content: '\25B8'; color: var(--accent); font-size: 11px; margin-right: 7px; }
  .card h4.ph code { font-family: 'JetBrains Mono', monospace; font-size: 0.92em; }
  .card h4.ph + ul { padding-left: 38px; }
```

If the file's CSS uses `--cyan` instead of `--accent` as the primary accent (likely — the networking-guide family uses cyan), the `::before` rule should reference `var(--cyan)` (or whichever token is the primary accent in this file — confirm by grepping `--cyan` or `--accent` in the `:root` block first). Use the same token that `.sec-title` uses for its color.

- [ ] **Step 4: Strip the template's content (keep chrome only)**

Delete all `<div class="nav-group">…</div>` blocks inside `<nav class="sidebar">` and all `<div class="section">…</div>` blocks inside `<main>`. Leave the chrome intact: sidebar shell + brand block, main shell, theme toggle, the closing `<script>` block at end of `<body>`.

- [ ] **Step 5: Add placeholder welcome section so the page renders**

Inside `<main>`:

```html
<div id="welcome" class="section active">
  <div class="sec-title">EVPN-VXLAN → EVPN-SRv6</div>
  <p class="sec-sub">Migration guide — scaffold pending content.</p>
</div>
```

- [ ] **Step 6: Verify in browser (or structurally via grep)**

Open the file. Verify:
- Page renders (no JS console errors)
- Dark/light mode toggle works
- Scaffold welcome section visible

Or via grep if no browser is available:
```bash
grep -c '<div id="welcome"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
grep -c '\.ph {' evpn-vxlan-to-srv6-migration-guide.html              # expect 1 (CSS rule)
grep -c '<nav class="sidebar"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
```

- [ ] **Step 7: Commit**

```bash
cd /Users/divakaran/arrcus_workspace/guides
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "feat(evpn-srv6): scaffold guide from mpls-to-srv6 template; port .ph CSS"
```

---

## Task 2: Build complete sidebar navigation

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Replace empty sidebar nav with all 25 sections + part dividers**

Insert inside `<nav class="sidebar">` after the `<div class="sidebar-brand">…</div>` block (and after the theme toggle if the template has it inside the sidebar):

```html
<div class="nav-group">
  <div class="nav-group-label">Front Matter</div>
  <button class="nav-item active" onclick="go('welcome',event)">Welcome</button>
  <button class="nav-item" onclick="go('migration-question',event)">The Migration Question</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 1 — Foundations</div>
  <button class="nav-item" onclick="go('stays-same',event)">1. What Stays the Same</button>
  <button class="nav-item" onclick="go('what-changes',event)">2. What Changes</button>
  <button class="nav-item" onclick="go('ref-topology',event)">3. Reference Topology</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 2 — Comparison</div>
  <button class="nav-item" onclick="go('header-math',event)">4. Header Math</button>
  <button class="nav-item" onclick="go('route-types',event)">5. Route-Type Behavior</button>
  <button class="nav-item" onclick="go('walk-type2',event)">6. Walk: Type 2 MAC/IP</button>
  <button class="nav-item" onclick="go('walk-type5',event)">7. Walk: Type 5 IP Prefix</button>
  <button class="nav-item" onclick="go('walk-bum',event)">8. Walk: Type 3 IMET / BUM</button>
  <button class="nav-item" onclick="go('hw-scale',event)">9. Hardware, Scale, MTU, Tooling</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 3 — Migration Playbook</div>
  <button class="nav-item" onclick="go('coex-baseline',event)">10. Coexistence Baseline</button>
  <button class="nav-item" onclick="go('coex-services',event)">11. Coexistence Per-Service</button>
  <button class="nav-item" onclick="go('gw-design',event)">12. Gateway Design</button>
  <button class="nav-item" onclick="go('gw-stitching',event)">13. Route-Type Stitching</button>
  <button class="nav-item" onclick="go('gw-esi-mac',event)">14. ESI, MAC Mobility, ARP/ND</button>
  <button class="nav-item" onclick="go('gw-walks',event)">15. Gateway Packet Walks</button>
  <button class="nav-item" onclick="go('mig-order',event)">16. Migration Order &amp; Approach</button>
  <button class="nav-item" onclick="go('mig-playbooks',event)">17. Service Cutover Playbooks</button>
  <button class="nav-item" onclick="go('final-cutover',event)">18. Final Cutover &amp; Cleanup</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Part 4 — Operational</div>
  <button class="nav-item" onclick="go('troubleshooting',event)">19. Troubleshooting</button>
  <button class="nav-item" onclick="go('monitoring',event)">20. Monitoring &amp; Telemetry</button>
</div>

<div class="nav-group">
  <div class="nav-group-label">Reference</div>
  <button class="nav-item" onclick="go('decision-matrix',event)">21. Decision Matrix</button>
  <button class="nav-item" onclick="go('cheat-sheet',event)">22. Cheat Sheet</button>
  <button class="nav-item" onclick="go('glossary',event)">23. Glossary</button>
</div>
```

- [ ] **Step 2: Verify sidebar renders correctly**

```bash
grep -c 'class="nav-group"' evpn-vxlan-to-srv6-migration-guide.html   # expect 6
grep -c 'class="nav-item' evpn-vxlan-to-srv6-migration-guide.html      # expect 25
grep -oE "go\('[a-z0-9-]+'" evpn-vxlan-to-srv6-migration-guide.html | sort -u | wc -l    # expect 25
```

Only `welcome` exists at this point; clicking other nav-items will leave the content area blank (expected, not a regression).

- [ ] **Step 3: Commit**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "feat(evpn-srv6): build complete sidebar navigation (25 sections)"
```

---

## Task 3: Front matter — §FM-1 Welcome + §FM-2 The Migration Question

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`
- Read: spec §FM-1 + §FM-2 briefs in `docs/superpowers/specs/2026-05-31-evpn-vxlan-to-srv6-migration-design.md`

- [ ] **Step 1: Replace scaffold welcome with full content**

Replace the placeholder `<div id="welcome">…</div>` with a complete welcome section covering:

- **Who this is for** — operators with a live EVPN-VXLAN fabric migrating to SRv6; architects choosing between encaps.
- **Prereqs** — assumes the reader has read or is fluent in: `bgp-evpn-guide.html` (BGP-EVPN with VXLAN), `dc-fabric-design-guide.html` (leaf-spine), `srv6-complete-guide.html` (SRv6 + uSID).
- **How to read** — textbook path (front to back) vs reference path (jump to §5 for route-type table, §22 for cheat sheet, §23 for glossary).
- **What's in the guide** — quick overview of the four Parts and the 5 packet walks.

Use the networking-guide voice (terse, opinionated, packed). Wrap in a single `<div class="card">` with `<div class="sec-title">Welcome</div>` and `<p class="sec-sub">…</p>` lede above the card.

**Add a `.flv-grid` (4 cards)** as a "what's in this guide" mini-tour:
- Comparison (Part 2): VXLAN vs SRv6 for EVPN, side-by-side packet walks
- Playbook (Part 3): coexistence → gateway → per-service → cutover
- Operational (Part 4): troubleshooting + monitoring across encaps
- Reference: decision matrix, cheat sheet, glossary

- [ ] **Step 2: Add §FM-2 The Migration Question section**

Insert after `<div id="welcome">…</div>`:

```html
<div id="migration-question" class="section">
  <div class="sec-title">The Migration Question</div>
  <p class="sec-sub">Why move a perfectly working VXLAN fabric to SRv6?</p>
  …
</div>
```

Cover (point-wise, with `<h4 class="ph">` per idea):
- ▸ **Header tax and scale** — SRv6 (esp. uSID) has lower overhead in many cases, and the SID space is effectively unlimited.
- ▸ **Unifying SP + DC underlay** — same IPv6/SRv6 underlay for both DC and WAN/SP networks.
- ▸ **Service insertion and traffic engineering** — SR Policy / TE constructs are first-class with SRv6; harder to bolt onto VXLAN.
- ▸ **Vendor / silicon roadmap pressure** — new ASIC generations are SRv6-first; VXLAN-only is becoming legacy in some product lines.
- ▸ **What's NOT a reason to migrate** — if VXLAN works at your scale and there's no SP-integration story, the migration may not pay off (cross-link §21 decision matrix).

End with an `<div class="info">` callout: "This guide assumes you've decided to migrate (or you're seriously considering it). If you haven't, jump to §21 first."

- [ ] **Step 3: Verify**

```bash
grep -c 'id="welcome"' evpn-vxlan-to-srv6-migration-guide.html         # expect 1
grep -c 'id="migration-question"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
grep -c 'class="ph"' evpn-vxlan-to-srv6-migration-guide.html           # expect ≥5 (the §FM-2 mini-headings)
```

- [ ] **Step 4: Commit**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §FM-1 Welcome + §FM-2 The Migration Question"
```

---

## Task 4: Part 1 Foundations — §1 + §2 + §3

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`
- Read: spec §1, §2, §3 briefs

- [ ] **Step 1: §1 — What stays the same (BGP-EVPN control plane)**

Insert after `<div id="migration-question">…</div>`:

Section scaffold: `<div id="stays-same" class="section">` + `<div class="sec-title">1. What Stays the Same — BGP-EVPN Control Plane</div>` + `<p class="sec-sub">The control plane is invariant; the data plane is what changes.</p>`.

Lede paragraph (prose, 1–2 sentences): frame the message that the migration doesn't touch EVPN config beyond next-hop / encap encoding.

Then `<div class="card">` with point-wise body:
- ▸ **BGP-EVPN AFI/SAFI is unchanged** — bullets: same SAFI (70), same RD/RT semantics, same RR design.
- ▸ **Route types still mean the same thing** — bullets: Type 2 (MAC/IP), Type 3 (IMET), Type 4 (ES route), Type 5 (IP Prefix); same purpose, only the encap-specific attributes differ. (Type 1 is covered in §5.)
- ▸ **MAC/IP learning is unchanged** — bullets: bridge-domain learning, ARP/ND suppression, MAC mobility procedures all identical.
- ▸ **EVPN config shape** — bullets: VRF / MAC-VRF definitions don't change; only the underlay forwarding context under each evpn-instance differs.

Visual: one `.flv-grid` with 4 cards summarizing what's preserved (AFI/SAFI · Route types · Learning · Config shape).

**Boundary note per the spec §5 scope clarifier:** "§1 names the types and confirms they survive the migration. §5 (Part 2) compares per-encap attributes for each type."

- [ ] **Step 2: §2 — What changes (encap, endpoint, service ID)**

Insert after §1:

Section scaffold: `<div id="what-changes" class="section">` + `<div class="sec-title">2. What Changes — Encap, Endpoint, Service ID</div>` + `<p class="sec-sub">Three layers of change: the encapsulation header, the endpoint identity, and the service identifier.</p>`.

Point-wise body:
- ▸ **Encapsulation header** — bullets: VXLAN (8B fixed inside UDP/IP) → SRv6 (IPv6 + optional SRH with segment list).
- ▸ **Endpoint identity** — bullets: VTEP (IPv4/v6 loopback) → SRv6 endpoint (uN locator block).
- ▸ **Service identifier** — bullets: VNI (24-bit) → SRv6 service SID (`uDT2U` for L2 VNIs, `uDT4`/`uDT6` for L3 VRFs in uSID terms; standard SRv6 spelling is `End.DT2U`/`End.DT4`/`End.DT6`).
- ▸ **Where uSID fits** — bullets: uSID compression collapses the segment list into the destination IPv6 address; "no SRH" for many EVPN flows.

Visuals:
- `<div class="cmp">` two-column: VXLAN side (header diagram, VTEP, VNI bullets) vs SRv6 side (IPv6+SRH, endpoint SID, service SID bullets). Each `<div class="cmp-c">` with `<h4>` header in the corresponding color (`var(--blue)` for VXLAN, `var(--orange)` or `var(--accent)` for SRv6).
- `<div class="pkt">` byte-box diagrams showing the two outer encapsulations stacked vertically for direct comparison. Use `bg-ipv`, `bg-pay`, `bg-srh`, `bg-vpn` segment colors.

- [ ] **Step 3: §3 — Reference topology**

Insert after §2:

Section scaffold: `<div id="ref-topology" class="section">` + `<div class="sec-title">3. Reference Topology</div>` + `<p class="sec-sub">Used in every packet walk and migration scenario. Read once, refer back as needed.</p>`.

Point-wise body:
- ▸ **Fabric** — bullets: 2-spine / 4-leaf, eBGP underlay (or iBGP-EVPN with RR on spines — explicitly state which we're using; the guide standardizes on RR-on-spines).
- ▸ **Hosts and services** — bullets: Host-A on Leaf-1 (VLAN-100), Host-B on Leaf-3 (VLAN-100), Host-C on Leaf-1 (VLAN-200), Host-D on Leaf-4 (VLAN-200, multi-homed to Leaf-3 and Leaf-4 via ESI). One L3 VRF spans both VLANs (IRB).
- ▸ **Three states** — bullets: State A (pure VXLAN, "before"); State B (pure SRv6 with uSID, "after"); State M (mid-migration, dual-encap with gateway at Leaf-2 and Leaf-4 as redundant border-leaves).
- ▸ **Address conventions** — bullets: leaf loopbacks `203.0.113.{1-4}` (v4) / `2001:db8:1::{1-4}/128` (v6); uN locator block `fc00:0:1::/48`; service SIDs `fc00:0:1:1:e000::` (uDT2U VNI-10100), `fc00:0:1:1:e001::` (uDT2U VNI-10200), `fc00:0:1:1:e100::` (uDT4 L3 VRF).

Visuals:
- Inline topology diagram (use a `<div class="pkt"…>` or pre-formatted ASCII block within `<div class="info">` for the topology; the existing networking guides use either approach. Look at `dc-fabric-design-guide.html` for a topology-diagram pattern to copy.).
- `<div class="flv-grid">` with 3 cards: State A · State B · State M, each with the one-line description.

- [ ] **Step 4: Verify**

```bash
grep -c 'id="stays-same"\|id="what-changes"\|id="ref-topology"' evpn-vxlan-to-srv6-migration-guide.html  # expect 3
```

- [ ] **Step 5: Commit**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write Part 1 Foundations (§1, §2, §3)"
```

---

## Task 5: §4 Header math + §5 Route-type behavior

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: §4 — Header math (VXLAN stack vs SRv6 stack)**

Section scaffold: `<div id="header-math" class="section">` + `<div class="sec-title">4. Header Math — VXLAN Stack vs SRv6 Stack</div>` + `<p class="sec-sub">Every migration starts by understanding the bytes you're swapping.</p>`.

Point-wise body:
- ▸ **VXLAN encap stack** — bullets: outer Eth + outer IP (20B v4 / 40B v6) + UDP (8B) + VXLAN (8B) + original Eth frame. Total fixed overhead: ~50B (v4 outer) or ~70B (v6 outer).
- ▸ **SRv6 encap stack** — bullets: outer Eth + outer IPv6 (40B) + optional SRH (8B + 16B × N segments) + payload. Without SRH (uSID single-locator case): 40B. With SRH and 1 segment: ~64B.
- ▸ **Header-tax comparison** — bullets summarizing best/worst case; jumbo-MTU implications (1500B host MTU vs 9000B fabric MTU).
- ▸ **What's the same across both** — bullets: original Eth frame for L2, original IP packet for L3 — neither encap mutates the payload.

Visuals:
- 3 `<div class="pkt">` byte-box diagrams stacked: VXLAN encap, SRv6-with-SRH, SRv6-no-SRH (uSID). Use `<div class="pkt-c bg-X">` segments with byte-count labels.
- `<table class="tbl">` header-tax matrix: rows = encap variant (VXLAN+v4outer, VXLAN+v6outer, SRv6 uSID, SRv6 with-SRH-1-seg, SRv6 with-SRH-2-seg); columns = outer header bytes / encap header bytes / total overhead.

- [ ] **Step 2: §5 — Route-type behavior across encaps**

Section scaffold: `<div id="route-types" class="section">` + `<div class="sec-title">5. Route-Type Behavior Across Encaps</div>` + `<p class="sec-sub">Same route types, different per-encap attributes.</p>`.

**Spec §5 scope boundary clarifier:** Open with a `<div class="info">` "Scope: §1 named the route types and confirmed they survive the migration. This section compares per-encap attributes for each. See §1 for the 'unchanged' summary."

Point-wise body — one `<h4 class="ph">` per route type, with bullets per encap:
- ▸ **Type 1 (Ethernet A-D, per-ES and per-EVI)** — bullets: carries ES Label (VXLAN) → ES SID (SRv6, e.g., a uN-derived value). Drives mass-withdrawal (per-ES) and aliasing (per-EVI). Same DF-election input on both encaps; only the label/SID encoding differs.
- ▸ **Type 2 (MAC / MAC+IP)** — bullets: MPLS Label1 → VNI (VXLAN) or service SID (SRv6, typically uDT2U / End.DT2U); Label2 (for IP+MAC) → L3 service SID; MAC mobility seq number carried identically.
- ▸ **Type 3 (Inclusive Multicast / IMET)** — bullets: PMSI Tunnel attribute (RFC 6514) signals replication mode; ingress-replication is the common mode for both; SRv6 has options VXLAN doesn't (e.g., native multicast via uDT2M / End.DT2M).
- ▸ **Type 4 (Ethernet Segment / ES route)** — bullets: ES Label (VXLAN) ↔ ES SID (SRv6); same DF-election algorithm.
- ▸ **Type 5 (IP Prefix)** — bullets: GW IP, GW MAC handling; label/SID differences; pure-L3 EVPN is the cleanest one to migrate first.

**The BGP attributes that carry the encap-specific data:**
- VXLAN side: BGP Encapsulation Extended Community (RFC 9012) with type-8 (VXLAN); MPLS Label fields in the EVPN NLRI repurposed as VNIs.
- SRv6 side: BGP Prefix-SID attribute (RFC 8669) with SRv6 Service TLV (RFC 9252) — Type 5 sub-TLV for L3, Type 6 sub-TLV for L2.

Add a `<div class="info">` callout: **BGP next-hop encoding gotcha** — if the receiving leaf doesn't support the sender's encap, the route is installed but unusable (silently); audit before going dual-stack.

Visuals:
- `<table class="tbl">` 5-row × 3-col: Route Type / VXLAN encoding / SRv6 (uSID) encoding.
- `<div class="info.orange">` for the next-hop gotcha callout above.

- [ ] **Step 3: Verify**

```bash
grep -c 'id="header-math"\|id="route-types"' evpn-vxlan-to-srv6-migration-guide.html  # expect 2
grep -c 'class="pkt"' evpn-vxlan-to-srv6-migration-guide.html  # expect ≥3 (the 3 byte-box diagrams)
```

- [ ] **Step 4: Commit**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §4 Header math + §5 Route-type behavior"
```

---

## Task 6: §6 Packet walk 1 — Type 2 MAC/IP, side-by-side

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Add §6 section scaffold**

Insert after §5:

```html
<div id="walk-type2" class="section">
  <div class="sec-title">6. Packet Walk 1 — Type 2 MAC/IP, Side-by-Side</div>
  <p class="sec-sub">Host-A → Host-B unicast (same VNI, different leaves), traced in both encaps.</p>
  …
</div>
```

- [ ] **Step 2: Scenario + control plane** (point-wise)

Inside a `<div class="card">`:
- Lede: "Host-A on Leaf-1 sends a unicast frame to Host-B on Leaf-3. Same VLAN-100 / VNI-10100. MAC already learned."
- ▸ **Scenario** — bullets: Host-A on Leaf-1, Host-B on Leaf-3, same VNI/VLAN, Host-B's MAC already in Leaf-1's bridge-domain MAC table.
- ▸ **Control plane** — bullets: BGP-EVPN Type 2 advertisement from Leaf-3 for Host-B's MAC; what Leaf-1 installs (VXLAN: VTEP=Leaf-3's loopback, VNI=10100; SRv6: SRv6 endpoint=`fc00:0:1:3::/64` locator, service SID=`fc00:0:1:1:e000::`).

- [ ] **Step 3: VXLAN data-plane walk** (`.stp` + `.pkt`)

Inside a `<div class="cmp">` left column (`<div class="cmp-c">`):
- `<h4 style="color:var(--blue);">VXLAN data-plane</h4>`
- `<div class="stp">` step-flow with 4 numbered steps showing the packet at each hop:
  1. Host-A → Leaf-1 (original L2 frame)
  2. Leaf-1 encaps with VXLAN header (UDP/4789, VNI=10100), outer IP src=Leaf-1, dst=Leaf-3 VTEP
  3. Spine forwards based on outer IP (ECMP across spines)
  4. Leaf-3 decapsulates (matches VNI=10100 → bridge-domain), L2 lookup for Host-B's MAC, forwards to Host-B
- Inline `<div class="pkt">` byte-box diagram for the encapsulated frame at step 2.

- [ ] **Step 4: SRv6 data-plane walk** (`.stp` + `.pkt`)

In the right column of the same `<div class="cmp">`:
- `<h4 style="color:var(--orange);">SRv6 data-plane (uSID)</h4>`
- `<div class="stp">` with 4 steps:
  1. Host-A → Leaf-1 (original L2 frame)
  2. Leaf-1 encaps with SRv6: outer IPv6 dst = service SID `fc00:0:1:3:e000::` (Leaf-3's uDT2U for VNI-10100); no SRH since it's a single-locator uSID flow
  3. Spine forwards based on the IPv6 destination (longest-prefix match within the uN locator block)
  4. Leaf-3 matches the destination against its local uDT2U behavior, strips outer IPv6, performs L2 lookup, forwards to Host-B
- Inline `<div class="pkt">` byte-box diagram for the SRv6-encapped frame at step 2.

- [ ] **Step 5: What the side-by-side highlights** (point-wise)

After the `<div class="cmp">`:
- ▸ **Header tax delta** — bullets: VXLAN ~50B; SRv6-uSID ~40B (with v6 underlay parity, the SRv6 case is actually leaner).
- ▸ **What FIB lookup happens at egress** — bullets: VXLAN matches VNI in a separate table; SRv6 matches the destination IPv6 against a local SID-table entry that's a uDT2U behavior.

Add a `<div class="info">` callout: "Same forwarding outcome; different lookup model. The performance trade-off lives in the silicon, not the protocol — see §9."

- [ ] **Step 6: Verify**

```bash
grep -c 'id="walk-type2"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
grep -A1 'id="walk-type2"' evpn-vxlan-to-srv6-migration-guide.html | grep -c 'class="cmp"'  # expect 1 nested cmp
```

- [ ] **Step 7: Commit**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §6 Packet walk 1 — Type 2 MAC/IP side-by-side"
```

---

## Task 7: §7 Packet walk 2 — Type 5 IP Prefix, side-by-side

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

Same structural pattern as Task 6. Specifics:

- [ ] **Step 1: Scaffold**

```html
<div id="walk-type5" class="section">
  <div class="sec-title">7. Packet Walk 2 — Type 5 IP Prefix, Side-by-Side</div>
  <p class="sec-sub">L3 VPN tenant, two subnets on two leaves — the cleanest service to migrate first.</p>
  …
</div>
```

- [ ] **Step 2: Scenario + control plane**

- Lede: "Host-A (subnet 10.10.100.0/24, VRF green) on Leaf-1 sends an IP packet to a host on subnet 10.10.200.0/24 (same VRF green) on Leaf-3. No shared bridge-domain — pure L3."
- ▸ **Scenario** — bullets: Symmetric IRB pattern; routing at ingress and egress.
- ▸ **Control plane** — bullets: Type 5 IP Prefix advertisement from Leaf-3 for 10.10.200.0/24; GW IP / GW MAC fields handle the IRB-to-IRB hop; VXLAN attaches L3-VNI for VRF green; SRv6 attaches L3 service SID `fc00:0:1:3:e100::` (uDT4 for VRF green).

- [ ] **Step 3: VXLAN walk (left column)**

`.stp` 4 steps: ingress IRB → VXLAN encap (L3-VNI) → spine forward → egress IRB lookup and decap. Inline `.pkt` byte-box.

- [ ] **Step 4: SRv6 walk (right column)**

`.stp` 4 steps: ingress IRB → SRv6 encap (uDT4 service SID) → spine forward → egress uDT4 behavior (decap + VRF lookup). Inline `.pkt` byte-box.

- [ ] **Step 5: Why this is the "safe first cut"**

Inside `<div class="info green">` callout:
- No bridge-domain state to worry about (no MAC mobility, no BUM).
- IRB cutover affects only the VRF, not the broadcast domain.
- Rollback is trivial — re-advertise via the old encap, withdraw the new one.

- [ ] **Step 6: Verify + Step 7: Commit**

```bash
grep -c 'id="walk-type5"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §7 Packet walk 2 — Type 5 IP Prefix side-by-side"
```

---

## Task 8: §8 Packet walk 3 — Type 3 IMET / BUM

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + scenario**

```html
<div id="walk-bum" class="section">
  <div class="sec-title">8. Packet Walk 3 — Type 3 IMET / BUM</div>
  <p class="sec-sub">ARP/flood handling. Ingress replication is the common ground; SRv6 unlocks options VXLAN can't reach.</p>
</div>
```

Scenario: Host-A sends an ARP request for 10.10.100.5 in VLAN-100/VNI-10100. The frame must reach all VTEPs/SRv6-endpoints in the VNI.

- [ ] **Step 2: Ingress replication — both encaps** (point-wise + topology diagram)

- ▸ **The model** — bullets: PE replicates the frame N times (once per remote endpoint), sends N separate unicast packets. Same model on both encaps; only the per-copy encap header differs.
- ▸ **VXLAN-specific (legacy): PIM-based multicast underlay** — bullets: less common, briefly noted; flagged in `<div class="info yellow">`.
- ▸ **SRv6-specific: uDT2M / End.DT2M for in-fabric replication** — bullets: replication can happen at a designated replicator inside the fabric rather than at the source PE; reduces ingress fan-out.

Visuals:
- Topology diagram showing flood from Leaf-1 to Leaf-2/3/4 (a `<div class="info">` block containing pre-formatted ASCII, or use the existing networking-guide topology-diagram pattern).
- Two `<div class="pkt">` byte-box diagrams: one for the VXLAN-encapped replicated copy, one for the SRv6 uDT2M copy.

- [ ] **Step 3: Migration implication** (point-wise)

- ▸ **Default to ingress replication** — bullets: same model on both encaps, lowest operational risk.
- ▸ **If your VXLAN deployment uses PIM underlay** — bullets: migrating to SRv6 is also a chance to drop PIM; plan the underlay-multicast retirement separately.

Add a `<div class="info yellow">` callout for the PIM caveat.

- [ ] **Step 4: Verify + Step 5: Commit**

```bash
grep -c 'id="walk-bum"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §8 Packet walk 3 — Type 3 IMET / BUM"
```

---

## Task 9: §9 Hardware, silicon, scale, MTU, operational tooling

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="hw-scale" class="section">
  <div class="sec-title">9. Hardware, Silicon, Scale, MTU, Operational Tooling</div>
  <p class="sec-sub">Where the comparison stops being protocol theory and starts being a buying decision.</p>
</div>
```

Point-wise body (one `<h4 class="ph">` per dimension):
- ▸ **Silicon support** — bullets: which ASIC families do VXLAN encap/decap at line rate vs which do SRv6 (incl. uSID); the "do my existing leaves support SRv6?" question; vendor-specific notes (Broadcom Trident, Tomahawk, Jericho; Cisco Silicon One; Mellanox/NVIDIA Spectrum).
- ▸ **Scale** — bullets: VNI count (24-bit = 16M theoretical, vendor-limited to lower); SRv6 SID space (huge — locator + function gives effectively unlimited service IDs); underlay routing-table sizing changes (BGP-LU/SR-MPLS legacy vs IPv6 underlay).
- ▸ **MTU implications** — bullets: header overhead per encap (from §4); how much fabric MTU headroom each needs (1500 host MTU + 50–70B for VXLAN, +40–64B for SRv6 → typical 9000 fabric MTU has plenty either way, but verify).
- ▸ **Operational tooling** — bullets: show commands (same shape for BGP-EVPN, different for encap-specific counters), telemetry counters, packet-capture filtering (UDP/4789 for VXLAN vs IPv6 routing-header / IPv6 dst-in-uN-locator for SRv6).
- ▸ **What stays the same operationally** — bullets: BGP-EVPN show commands, MAC table, VRF show commands.

Visuals:
- `<table class="tbl">` matrix: rows = silicon / scale / MTU / tooling; columns = VXLAN / SRv6 (uSID).
- `<div class="flv-grid">` with 4 cards for the sharpest deltas (1 card per real difference).

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="hw-scale"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §9 Hardware, scale, MTU, tooling"
```

---

## Task 10: Phase 1 Coexistence — §10 + §11

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: §10 — Coexistence baseline (dual-stack fabric design)**

Scaffold: `<div id="coex-baseline" class="section">` + `<div class="sec-title">10. Coexistence Baseline — Dual-Stack Fabric Design</div>` + lede ("Coexistence isn't transient — operators sit in it for months.").

Point-wise body:
- ▸ **What dual-stack means here** — bullets: same physical fabric advertises both VXLAN and SRv6 EVPN; leaves can be VXLAN-only, SRv6-only, or both; control plane carries both via BGP Encapsulation Ext Community + BGP Prefix-SID + SRv6 Service TLV.
- ▸ **Underlay requirements** — bullets: SRv6 needs IPv6 reachability (loopback-per-leaf in v6); IPv4 underlay can stay for VXLAN; uN locator block allocation (using the reference topology's `fc00:0:1::/48`).
- ▸ **Control-plane design** — bullets: RR config carries both AFI/SAFI; next-hop-self vs next-hop-unchanged at border; iBGP-EVPN vs eBGP-EVPN tradeoffs.
- ▸ **Pre-migration sanity checks** — bullets: no duplicate VNI/SID semantics, MTU headroom verified, hardware support audit per leaf.

Visuals:
- Topology diagram (State M from §3): dual-encap fabric with mixed-capability leaves and gateway at Leaf-2/Leaf-4.
- `<div class="cmp">` two-column: VXLAN-side BGP attributes (Encap-Type 8 = VXLAN; MPLS Label1 = VNI) vs SRv6-side (Encap-Type N; SRv6 Service TLV in BGP Prefix-SID).
- `<div class="info yellow">` callout: **the next-hop encoding gotcha** — if a leaf advertises an SRv6 next-hop and a peer doesn't understand SRv6, the route gets installed but is silently unusable.

- [ ] **Step 2: §11 — Coexistence per-service callouts**

Scaffold: `<div id="coex-services" class="section">` + sec-title "11. Coexistence — What It Means Per Service".

Point-wise: one `<h4 class="ph">` per service area, with bullets per service describing what to watch during coexistence:
- ▸ **L2 (Type 2)**, ▸ **L3 (Type 5)**, ▸ **IRB (symmetric)**, ▸ **Multi-homing (ESI / Type 4 + Type 1)** — bullets per the spec §11 brief.

For the 5th: the **DCI card is included as a deployment-topology cross-cut**, NOT a peer service — be explicit in the card title and body text:
- ▸ **DCI (deployment-topology cross-cut)** — bullets: site-to-site interconnect can stay VXLAN, become SRv6, or be the gateway boundary itself; matters most for multi-site fabrics. Not a service category like L2/L3 — it's a topology dimension shown here for navigation.

Visuals:
- `<div class="flv-grid">` with 5 cards in this order: L2 · L3 · IRB · Multi-homing · DCI (deployment-topology cross-cut). Each card has a one-line "what to watch."
- `<div class="info">` callout: "Pick one service to lead the migration — typically L3 / Type 5. See §16 for the recommended order."

- [ ] **Step 3: Verify + Step 4: Commit**

```bash
grep -c 'id="coex-baseline"\|id="coex-services"' evpn-vxlan-to-srv6-migration-guide.html  # expect 2
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write Phase 1 Coexistence (§10, §11)"
```

---

## Task 11: §12 Gateway design

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="gw-design" class="section">
  <div class="sec-title">12. Gateway Design — Border-Leaf vs Spine-as-Gateway vs Dedicated DCI</div>
  <p class="sec-sub">The gateway is where two data planes touch. Placement determines blast radius, scale, and rollback flexibility.</p>
</div>
```

Point-wise body:
- ▸ **Border-leaf as gateway** — bullets: most common; one or more leaves run dual-encap and stitch; isolates blast radius; easy to add/remove during migration.
- ▸ **Spine as gateway** — bullets: fewer devices to upgrade; but spines bought for throughput, not features — confirm SRv6 support per silicon.
- ▸ **Dedicated DCI router** — bullets: appropriate for inter-fabric (multi-site) cases; can be different vendor than the leaves.
- ▸ **Hardware checklist** — bullets: line-rate VXLAN encap/decap + SRv6 encap/decap; route-type translation is control-plane (any modern router with EVPN can do it).
- ▸ **HA design** — bullets: gateway pair with anycast next-hop OR active/standby with state sync; failure-mode considerations.

Visuals:
- `<div class="cmp">` three-column (border-leaf · spine-as-gateway · dedicated DCI) with strengths/weaknesses per column.
- Inline mini-topology diagrams showing each placement (use small ASCII boxes inside `<div class="info">` or `<div class="pkt">` style).
- `<table class="tbl">` decision matrix: rows = fabric size / multi-site? / vendor mix; columns = recommended gateway placement.

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="gw-design"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §12 Gateway design"
```

---

## Task 12: §13 Route-type stitching mechanics

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="gw-stitching" class="section">
  <div class="sec-title">13. Route-Type Stitching Mechanics</div>
  <p class="sec-sub">The gateway is a stitching / re-origination PE — intra-fabric, not an ASBR. Same AS, encap boundary.</p>
</div>
```

**Explicit framing note per spec L1 fix:** open with a `<div class="info">` callout: "The gateway is functionally an EVPN re-origination PE within the same AS. The term 'ASBR' (Autonomous System Boundary Router) implies an inter-AS boundary and does not apply here — the gateway sits at an encap boundary, not an AS boundary."

Point-wise body:
- ▸ **The stitching pattern** — bullets: ingress receives EVPN route in encap A; gateway re-originates with its own next-hop; encap-specific attributes (label/VNI/SID) are translated; RD/RT preserved (or rewritten if policy demands).
- ▸ **Type 1 (ES A-D) stitching** — bullets: ES Label ↔ ES SID translation; aliasing semantics maintained across encaps.
- ▸ **Type 2 (MAC/IP) stitching** — bullets: VNI ↔ uDT2U/End.DT2U service SID; MAC stays the same; gateway maintains MAC mobility seq number consistently.
- ▸ **Type 3 (IMET) stitching** — bullets: PMSI Tunnel attribute (RFC 6514) rewritten per encap; replication targets re-computed.
- ▸ **Type 4 (ES route) stitching** — bullets: ES Label ↔ ES SID; DF election runs on the combined view; gateway is NOT a DF candidate.
- ▸ **Type 5 (IP Prefix) stitching** — bullets: simplest case; GW IP / GW MAC rewritten; label ↔ uDT4/End.DT4 swap.

Visuals:
- `<div class="stp">` generic stitching pipeline: (1) receive in encap A → (2) translate per-route-type attrs → (3) re-originate with new next-hop in encap B.
- `<table class="tbl">` route-type translation matrix: row per Type 1–5; columns = "VXLAN side encoding" / "SRv6 side encoding" / "what the gateway changes."
- `<div class="info orange">` callout: "The gateway must run BGP-EVPN sessions BOTH as a VXLAN PE AND as an SRv6 PE — it's two PEs in one box, glued by a re-origination policy."

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="gw-stitching"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §13 Route-type stitching mechanics"
```

---

## Task 13: §14 ESI + MAC mobility + ARP/ND across encaps

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="gw-esi-mac" class="section">
  <div class="sec-title">14. ESI + MAC Mobility + ARP/ND Across Encaps</div>
  <p class="sec-sub">The operationally subtle hazards live here — not in single-flow forwarding, but in events.</p>
</div>
```

Point-wise body:
- ▸ **ESI consistency** — bullets: an Ethernet Segment spanning encap-A leaves and encap-B leaves must advertise the SAME ESI on both sides; otherwise DF election is wrong; gateway does not invent ESIs.
- ▸ **MAC mobility events** — bullets: host moves from VXLAN leaf to SRv6 leaf → new Type 2 with incremented seq number; gateway MUST propagate the seq number; re-originating without it breaks mobility (causes MAC flap loops).
- ▸ **ARP/ND across the encap boundary** — bullets: ARP suppression caches must be coherent; gateway should proxy-ARP where the target is in the other encap's bridge domain, OR rely on flood-and-respond if proxy isn't safe.
- ▸ **Symmetric IRB consistency** — bullets: anycast gateway MAC + IP must be identical on both encaps; if they drift, hosts learn wrong gateway after a move.
- ▸ **Common failures** — bullets: MAC flap loop via seq-number mismatch; duplicate-address detection storms; silent-host visibility gaps.

Visuals:
- `<div class="stp">` MAC mobility sequence: host moves → new Type 2 → gateway sees both old + new → propagates with seq number → both sides converge.
- `<div class="info red">` callout listing the 3 most common operational pitfalls.

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="gw-esi-mac"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §14 ESI + MAC mobility + ARP/ND across encaps"
```

---

## Task 14: §15 Gateway packet walks (forward + return)

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

This is **Packet walks 4 and 5** — the gateway crossing in both directions.

- [ ] **Step 1: Scaffold + scenario**

```html
<div id="gw-walks" class="section">
  <div class="sec-title">15. Gateway Packet Walks — VXLAN Host ↔ SRv6 Host</div>
  <p class="sec-sub">A packet that crosses the encap boundary. Forward + return.</p>
</div>
```

Lede: "Host-A is on a VXLAN leaf (Leaf-1). Host-B is on an SRv6 leaf (Leaf-3). Same VNI / mapped service SID. The gateway (Leaf-2 in State M) translates."

- [ ] **Step 2: Walk 4 — forward path (VXLAN host → Gateway → SRv6 host)**

`<div class="cmp">` left column — `<h4 style="color:var(--blue);">Forward (VXLAN → SRv6)</h4>`:
- `<div class="stp">` 5 numbered steps:
  1. Host-A → Leaf-1 (original L2 frame)
  2. Leaf-1 VXLAN-encaps (VTEP dst = Gateway/Leaf-2; VNI=10100)
  3. Gateway receives; decapsulates VXLAN; the bridge-domain MAC table maps Host-B to SRv6 endpoint `fc00:0:1:3::/64` + service SID `e000::`
  4. Gateway SRv6-encaps (outer IPv6 dst = `fc00:0:1:3:e000::`)
  5. Leaf-3 receives; matches uDT2U behavior; decaps; forwards to Host-B
- Inline `<div class="pkt">` byte-box at step 2 (VXLAN frame) AND at step 4 (SRv6 frame).

- [ ] **Step 3: Walk 5 — return path (SRv6 host → Gateway → VXLAN host)**

Right column — `<h4 style="color:var(--orange);">Return (SRv6 → VXLAN)</h4>`:
- `<div class="stp">` 5 numbered steps (the mirror):
  1. Host-B → Leaf-3
  2. Leaf-3 SRv6-encaps (outer IPv6 dst = Gateway's service SID for VNI-10100)
  3. Gateway receives; matches uDT2U; decaps; bridge-domain MAC table maps Host-A to Leaf-1's VTEP
  4. Gateway VXLAN-encaps
  5. Leaf-1 receives; VXLAN-decaps; forwards to Host-A
- Inline `.pkt` byte-boxes for both encaps.

- [ ] **Step 4: What the walks expose** (point-wise)

After the `<div class="cmp">`:
- ▸ **Header bumps twice per direction** — bullets: VXLAN encap → decap+SRv6 encap → SRv6 decap → SRv6 encap (return) → decap+VXLAN encap → VXLAN decap. Six encap operations per round-trip.
- ▸ **MTU implications at the gateway** — bullets: gateway must handle the largest of (host MTU + VXLAN overhead) and (host MTU + SRv6 overhead); fabric MTU must accommodate both encap headers.
- ▸ **Asymmetric routing risk** — bullets: forward and return *should* take the same path through the same gateway; misconfigured policies can split them, breaking stateful services (firewalls, load balancers).

Add `<div class="info">` callout: "Always test forward+return through the same gateway BEFORE introducing per-service migration."

- [ ] **Step 5: Verify + Step 6: Commit**

```bash
grep -c 'id="gw-walks"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §15 Gateway packet walks (walks 4+5)"
```

---

## Task 15: Phase 3 Per-service migration — §16 + §17

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: §16 — Migration order and approach**

Scaffold: `<div id="mig-order" class="section">` + sec-title "16. Migration Order & Approach".

Point-wise body:
- ▸ **Recommended order** — bullets: pure L3 (Type 5) → IRB → L2 single-homed → L2 multi-homed → DCI. Reason per step (less state, less risk, easier rollback).
- ▸ **Cutover patterns** — bullets: per-VRF (cleanest, isolates failure), per-VLAN (within a tenant), per-tenant (groups VRFs+VLANs), per-leaf-group (whole-leaf cutover; useful with hardware refresh).
- ▸ **Rollback design** — bullets: leave VXLAN config in place on each leaf until verified for all its tenants; per-service rollback should not require gateway reconfig.
- ▸ **Observability during cutover** — bullets: MAC table convergence, ARP table consistency, gateway counters, expected vs anomalous patterns.

Visuals:
- `<div class="stp">` 5-step flow showing the recommended order (use `stp-n grn` for "do now," `stp-n org` for "next," `stp-n pur` for "do last").
- `<div class="flv-grid">` with 4 cards for the cutover patterns.
- `<div class="info green">` callout: "Every per-service cutover should be reversible WITHOUT touching the gateway."

- [ ] **Step 2: §17 — Service-specific cutover playbooks**

Scaffold: `<div id="mig-playbooks" class="section">` + sec-title "17. Service-Specific Cutover Playbooks".

**Note the DCI framing per spec L-R2-1 fix:** open with `<div class="info">` callout: "The 5th playbook below is DCI, included as a migratable unit even though §11 framed DCI as a deployment-topology cross-cut. Operators in practice cut over inter-site connectivity as a discrete step — this playbook captures that."

One `<h4 class="ph">` per service, with bullets per service:
- ▸ **L3 / Type 5 cutover** — bullets: pre-checks (VRF count, prefix count) → cutover (advertise via SRv6, verify routing, withdraw VXLAN) → validation → rollback.
- ▸ **IRB cutover** — bullets: anycast gateway consistency check first → symmetric IRB cutover → validation → rollback.
- ▸ **L2 single-homed cutover** — bullets: VNI ↔ service SID mapping verified → MAC table snapshot before/after → ARP suppression cache flush → rollback.
- ▸ **L2 multi-homed (ESI) cutover** — bullets: DF election stability under encap-A → dual → encap-B; the trickiest cut; rollback.
- ▸ **DCI cutover** — bullets: site-by-site or gateway-to-gateway; depends on inter-site fabric; usually last.

Visuals:
- `<div class="flv-grid">` with 5 cards at the top as a navigation index.
- For each service, a mini `<div class="stp">` (3–4 steps) for the cutover sequence.
- `<table class="tbl">` per service or one combined: pre-check / cutover / validation / rollback columns.

- [ ] **Step 3: Verify + Step 4: Commit**

```bash
grep -c 'id="mig-order"\|id="mig-playbooks"' evpn-vxlan-to-srv6-migration-guide.html  # expect 2
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write Phase 3 Per-service migration (§16, §17)"
```

---

## Task 16: §18 Final cutover, rollback, cleanup

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="final-cutover" class="section">
  <div class="sec-title">18. Final Cutover, Rollback, Cleanup</div>
  <p class="sec-sub">The gateway is a crutch; eventually you remove it.</p>
</div>
```

Point-wise body:
- ▸ **When the gateway can retire** — bullets: all services migrated; no encap-A advertisements remaining; observability green for N days/weeks.
- ▸ **Gateway decommissioning steps** — bullets: withdraw encap-A from gateway → verify no traffic on the old encap → remove dual-encap config → optionally repurpose the device as a regular leaf.
- ▸ **VXLAN-specific cleanup** — bullets: remove unused VTEP loopbacks; unused IPv4 underlay if SRv6 is now the only data plane; unused PIM/multicast underlay (if used).
- ▸ **Post-migration validation** — bullets: end-to-end test plan per service; before/after counter comparison; scale-number verification.
- ▸ **What to keep around** — bullets: dual-stack-period docs (for audit/postmortem); "if SRv6 fails, can we roll back?" runbook (yes for weeks, no after deep cleanup).

Visuals:
- `<div class="stp">` showing decom sequence (retire gateway → cleanup VXLAN → cleanup IPv4 underlay).
- `<div class="info orange">` callout: "Rollback AFTER deep cleanup means redeploying VXLAN — that's a project of its own, not a one-day operation."

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="final-cutover"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §18 Final cutover, rollback, cleanup"
```

---

## Task 17: Part 4 Operational — §19 + §20

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: §19 — Troubleshooting**

Scaffold: `<div id="troubleshooting" class="section">` + sec-title "19. Troubleshooting — Control Plane vs Data Plane".

Point-wise body:
- ▸ **What stays the same** — bullets: `show bgp evpn`, `show evpn mac`, `show evpn arp`, `show ip route vrf` (same shape, same fields).
- ▸ **Control-plane diffs per encap** — bullets: VXLAN shows VNI + VTEP IP in next-hop; SRv6 shows service SID + locator/endpoint; how to read each.
- ▸ **Data-plane capture: VXLAN** — bullets: filter UDP/4789; 8-byte VXLAN header; VNI in plaintext.
- ▸ **Data-plane capture: SRv6** — bullets: filter IPv6 routing header (next-header 43); SRH parsing in modern tcpdump/Wireshark; uSID compression means the function is in the destination address.
- ▸ **Common failure modes** — bullets: MAC mobility loop, wrong next-hop install, ESI mismatch, MTU drops at gateway, missing IPv6 underlay route. Per failure: control-plane symptom → data-plane symptom → fix.
- ▸ **Gateway-specific failures** — bullets: stuck routes (gateway didn't re-originate); one-way traffic (asymmetric encap); state desync after gateway failover.

Visuals:
- `<div class="cmp">` two-column: VXLAN debug toolkit vs SRv6 debug toolkit (commands, capture filters, decoders).
- `<table class="tbl">` failure × symptom × fix matrix.
- `<div class="info red">` callout for the "MAC table looks fine but no traffic" classic (usually MTU at the gateway).

- [ ] **Step 2: §20 — Monitoring, telemetry, observability**

Scaffold: `<div id="monitoring" class="section">` + sec-title "20. Monitoring, Telemetry, Observability".

Point-wise body:
- ▸ **Counter mapping** — bullets: per-VNI counters → per-service-SID counters; locator-level vs function-level granularity; which YANG paths to subscribe to.
- ▸ **Flow export** — bullets: sFlow/IPFIX still works; flow records need new fields to capture SRH/SID; collector config changes.
- ▸ **SLA / probe-based monitoring** — bullets: STAMP / TWAMP / BFD over the underlay; BFD over SRv6 vs over VXLAN; sub-50ms detection achievable on both.
- ▸ **Dashboards during dual-stack** — bullets: split panels (VXLAN-side, SRv6-side); cross-encap visibility at the gateway is the only genuinely new view.
- ▸ **Alerting drift** — bullets: scale-counter thresholds (SRv6 SID growth shape differs); encap-mismatch alerts; gateway translation-failure alerts.

Visual: `<table class="tbl">` monitoring matrix: dimension × VXLAN / SRv6 / Gateway-specific. Plus `<div class="info">` callout about auditing custom dashboards for VNI assumptions before cutover.

- [ ] **Step 3: Verify + Step 4: Commit**

```bash
grep -c 'id="troubleshooting"\|id="monitoring"' evpn-vxlan-to-srv6-migration-guide.html  # expect 2
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write Part 4 Operational (§19, §20)"
```

---

## Task 18: §21 Decision matrix

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="decision-matrix" class="section">
  <div class="sec-title">21. Decision Matrix — Migrate Now / Later / Stay</div>
  <p class="sec-sub">Not every operator should migrate. Here are the signals.</p>
</div>
```

Point-wise body:
- ▸ **Migrate now if…** — bullets: SP+DC unified underlay goal; SRv6-capable silicon already deployed; scaling past VXLAN's practical VNI ceiling; multi-domain TE pulling DC traffic; vendor EOL on VXLAN-only platforms.
- ▸ **Migrate later if…** — bullets: hardware refresh due in 1–3 years; current scale and tooling work; no business pull; organizational SRv6 skill gap.
- ▸ **Stay on VXLAN if…** — bullets: single-site, modest scale; no SP integration story; no SRv6 expertise; hardware tied to one vendor without an SRv6 roadmap.

Visuals:
- `<table class="tbl">` decision matrix: signal × outcome (migrate now / later / stay).
- `<div class="flv-grid">` with 3 outcome cards (the strongest signal each).

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="decision-matrix"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §21 Decision matrix"
```

---

## Task 19: §22 Cheat sheet

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + content**

```html
<div id="cheat-sheet" class="section">
  <div class="sec-title">22. VXLAN vs SRv6 Cheat Sheet</div>
  <p class="sec-sub">One-page reference, dense, scan-only.</p>
</div>
```

Dense reference — use `<table class="tbl">` throughout (not point-wise prose):
- **Encap headers** — small side-by-side byte diagrams (no walks, just structure).
- **Common show commands** — table: command → VXLAN output / SRv6 output.
- **Route attribute quick reference** — BGP Encapsulation Extended Community values; BGP Prefix-SID structure with SRv6 Service TLV; PMSI Tunnel attribute.
- **Capture filters** — VXLAN (UDP/4789); SRv6 (IPv6 next-header 43 or IPv6 dst-in-uN-locator).
- **Common SID functions** — `uN`, `uA`, `uDT2U` / `End.DT2U`, `uDT2M` / `End.DT2M`, `uDT4` / `End.DT4`, `uDT6` / `End.DT6` — one line each.

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="cheat-sheet"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §22 Cheat sheet"
```

---

## Task 20: §23 Glossary

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html`

- [ ] **Step 1: Scaffold + glossary content**

```html
<div id="glossary" class="section">
  <div class="sec-title">23. Glossary</div>
  <p class="sec-sub">Terms specific to this guide. Adds to the bgp-evpn and srv6-complete glossaries.</p>

  <div class="card">
    <dl class="glossary">
      <dt><strong>BGP Encapsulation Extended Community</strong> (RFC 9012)</dt>
      <dd>BGP extended community signaling the data-plane encapsulation type for an EVPN route. Type 8 = VXLAN; other values for MPLS, SRv6, etc. The receiving PE uses this to decide whether to install the route in its data plane.</dd>

      <dt><strong>BGP Prefix-SID attribute</strong> (RFC 8669)</dt>
      <dd>BGP attribute (type 40) that carries Segment Routing identifiers. Originally defined for SR-MPLS; extended by RFC 9252 for SRv6 to carry service SIDs in EVPN and L3VPN routes via the SRv6 Service TLV.</dd>

      <dt><strong>SRv6 Service TLV</strong> (RFC 9252)</dt>
      <dd>Sub-TLV inside the BGP Prefix-SID attribute that carries an SRv6 service SID for a BGP route. Type 5 sub-TLV for L3 services, Type 6 sub-TLV for L2 services (EVPN).</dd>

      <dt><strong>ES Label / ES SID</strong></dt>
      <dd>Identifier used in EVPN multi-homing for split-horizon and DF election. In VXLAN encap it's an MPLS label carried in the EVPN ESI advertisement; in SRv6 it's a SID with split-horizon semantics.</dd>

      <dt><strong>End.DT2U</strong>, <strong>End.DT2M</strong>, <strong>End.DT4</strong>, <strong>End.DT6</strong></dt>
      <dd>SRv6 service-SID endpoint behaviors. End.DT2U: decap and L2 unicast lookup. End.DT2M: decap and L2 multicast/BUM lookup. End.DT4 / End.DT6: decap and IPv4/IPv6 VRF route lookup. uSID-flavor equivalents: uDT2U, uDT2M, uDT4, uDT6 (used in compressed-SRv6 deployments — see srv6-complete-guide for the mapping).</dd>

      <dt><strong>Gateway / border-leaf</strong></dt>
      <dd>In this guide, a leaf (or pair of leaves) that runs both VXLAN and SRv6 data planes simultaneously and stitches EVPN routes between them by re-originating with translated encap-specific attributes. Not an ASBR (intra-fabric, same AS).</dd>

      <dt><strong>IMET</strong> (Inclusive Multicast Ethernet Tag)</dt>
      <dd>EVPN Type 3 route, advertised per (EVI, Ethernet Tag) by each PE to signal it's a member of that bridge-domain and to convey replication-tree parameters via the PMSI Tunnel attribute.</dd>

      <dt><strong>PMSI Tunnel attribute</strong> (RFC 6514)</dt>
      <dd>BGP attribute carried in Type 3 (IMET) advertising the inclusive-multicast tunnel type and identifier. Used to signal ingress replication vs P2MP-tunnel vs PIM modes.</dd>

      <dt><strong>Route re-origination</strong></dt>
      <dd>What the gateway does when stitching encaps: receives an EVPN route in encap A, replaces the next-hop and encap-specific attributes (label/SID), and re-advertises into encap B's BGP-EVPN domain. RD/RT are usually preserved.</dd>

      <dt><strong>Service SID</strong></dt>
      <dd>An SRv6 SID that maps to a service-specific endpoint behavior (End.DT*/uDT*). Carried in the BGP Prefix-SID attribute via the SRv6 Service TLV.</dd>

      <dt><strong>Stitching</strong></dt>
      <dd>Synonym for re-origination at an encap boundary. See "Route re-origination."</dd>

      <dt><strong>uN</strong> / <strong>uA</strong></dt>
      <dd>uSID (compressed SRv6) flavors of the End and End.X endpoint behaviors. uN is the node-locator end behavior (transit); uA is the adjacency end behavior (forward via specific link).</dd>

      <dt><strong>VNI ↔ Service SID mapping</strong></dt>
      <dd>The migration's central data-plane translation. A VXLAN VNI (24-bit) maps 1:1 to an SRv6 service SID (typically a uDT2U for an L2 VNI or a uDT4/uDT6 for an L3 VRF). The gateway maintains the mapping in its stitching policy.</dd>

      <dt><strong>VTEP / SRv6 endpoint equivalence</strong></dt>
      <dd>A VXLAN VTEP (a loopback IP that terminates VXLAN tunnels) is functionally equivalent to an SRv6 endpoint (a uN-locator-derived address that terminates SRv6 paths). Both are the "where the encap is decapped" address.</dd>
    </dl>
  </div>
</div>
```

- [ ] **Step 2: Verify + Step 3: Commit**

```bash
grep -c 'id="glossary"' evpn-vxlan-to-srv6-migration-guide.html  # expect 1
grep -c '<dt>' evpn-vxlan-to-srv6-migration-guide.html  # expect ~14 glossary terms
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "docs(evpn-srv6): write §23 Glossary"
```

---

## Task 21: Cross-link verification pass

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html` (only if fixes needed)

- [ ] **Step 1: List all referenced ids**

```bash
cd /Users/divakaran/arrcus_workspace/guides
grep -oE "go\('[a-z0-9-]+'" evpn-vxlan-to-srv6-migration-guide.html | sed -E "s/go\('([a-z0-9-]+)'/\1/" | sort -u > /tmp/evpn-refs.txt
cat /tmp/evpn-refs.txt
```

- [ ] **Step 2: Cross-check against actual section ids**

```bash
grep -oE '<div[^>]*id="[a-z0-9-]+"' evpn-vxlan-to-srv6-migration-guide.html | sed -E 's/.*id="([^"]+)"/\1/' | sort -u > /tmp/evpn-ids.txt
comm -23 /tmp/evpn-refs.txt /tmp/evpn-ids.txt   # refs with no matching id = orphan
comm -13 /tmp/evpn-refs.txt /tmp/evpn-ids.txt   # ids with no incoming reference = unreferenced section (warn but not always wrong)
```

Expected: first `comm` (orphans) is empty. Second `comm` may contain `welcome` (the landing has no incoming go() call until it's clicked).

- [ ] **Step 3: Fix any orphans, recommit if changed**

If orphans found, rename either the reference or the id to match. Commit:
```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "fix(evpn-srv6): cross-link verification — repair orphan see-also IDs"
```

---

## Task 22: Visual review pass

**Files:**
- Modify: `evpn-vxlan-to-srv6-migration-guide.html` (only if fixes needed)

- [ ] **Step 1: Open in default browser, walk every section**

```bash
open /Users/divakaran/arrcus_workspace/guides/evpn-vxlan-to-srv6-migration-guide.html
```

Check each of the 25 sections: layout doesn't break, code blocks don't overflow, tables don't blow out on narrow widths, packet diagrams render correctly.

- [ ] **Step 2: Toggle dark/light mode on every section**

Look for unreadable color combos (especially info boxes, packet-color segments, comparison cards, walkthrough boxes if any were added).

- [ ] **Step 3: Test on narrow width (~480px)**

Resize browser to ~480px. Check: sidebar collapse behavior matches template; `.flv-grid` cards wrap; `.cmp` two-column wraps; `.pkt` byte boxes don't overflow.

- [ ] **Step 4: Console check**

Open DevTools → Console. Click 5 random sidebar entries spanning different Parts. Confirm `console.error` count remains 0.

- [ ] **Step 5: Tag balance check**

```bash
python3 -c "
import re
h = open('evpn-vxlan-to-srv6-migration-guide.html').read()
for tag in ['div', 'table', 'pre', 'ul', 'aside']:
    o = len(re.findall(r'<'+tag+r'[ >]', h))
    c = len(re.findall(r'</'+tag+r'>', h))
    print(f'{tag}: {o}/{c} {\"OK\" if o==c else \"*** MISMATCH\"}')
"
```

All should be OK.

- [ ] **Step 6: Fix and commit (if any visual issues)**

```bash
git add evpn-vxlan-to-srv6-migration-guide.html
git commit -m "fix(evpn-srv6): visual polish"
```

---

## Task 23: README + IDEAS.md update + final commit

**Files:**
- Modify: `/Users/divakaran/arrcus_workspace/guides/README.md`
- Modify: `/Users/divakaran/arrcus_workspace/guides/IDEAS.md`

- [ ] **Step 1: Add the guide to README's "SRv6 Advanced & Migration" bucket**

In `README.md`, locate the `### SRv6 Advanced & Migration` section's table and add a new row:

```markdown
| EVPN-VXLAN to EVPN-SRv6 Migration Guide | [evpn-vxlan-to-srv6-migration-guide.html](evpn-vxlan-to-srv6-migration-guide.html) | Moving the DC overlay data plane from VXLAN to SRv6 — comparison + migration playbook. Header math, route-type behavior across encaps (Type 1/2/3/4/5), 5 side-by-side packet walks, gateway/border-leaf stitching, ESI + MAC mobility across encaps, per-service cutover playbooks (L3 → IRB → L2 → multi-homing → DCI), troubleshooting, monitoring, and a decision matrix. |
```

Add it as the last row in the "SRv6 Advanced & Migration" bucket (after the MPLS-to-SRv6 migration row).

- [ ] **Step 2: Mark IDEAS.md #6 as Done**

In `IDEAS.md`, locate the row:
```
| 6 | **EVPN-VXLAN to EVPN-SRv6 Migration** | Moving DC overlay from VXLAN to SRv6 data plane | Planned |
```
Change `Planned` → `Done`. Optionally add a brief note ("Done — shipped 2026-05-31 as `evpn-vxlan-to-srv6-migration-guide.html`.").

- [ ] **Step 3: Final commit**

```bash
git add README.md IDEAS.md
git commit -m "docs: add EVPN-VXLAN→EVPN-SRv6 Migration Guide to README; mark IDEAS #6 Done"
```

- [ ] **Step 4: Optional — open the finished guide one more time**

Click through 5–8 random chapters end-to-end. If anything feels rough, file a follow-up issue rather than fix in this PR.

---

## Self-Review Notes (writer to writer)

**Spec coverage:** Every section in spec §7 has a task here. §FM-1, §FM-2 → Task 3. §1–§3 → Task 4. §4–§9 → Tasks 5–9. §10–§11 → Task 10. §12 → Task 11. §13 → Task 12. §14 → Task 13. §15 → Task 14. §16–§17 → Task 15. §18 → Task 16. §19–§20 → Task 17. §21 → Task 18. §22 → Task 19. §23 → Task 20. All 5 packet walks accounted for (Walk 1 in Task 6, Walk 2 in Task 7, Walk 3 in Task 8, Walks 4–5 in Task 14). All spec framing rules carried into the relevant tasks: point-wise body throughout (every section task uses `.ph` mini-headings + bullets); §5 scope-boundary clarifier inserted in Task 5 Step 2; §11 DCI cross-cut framing in Task 10 Step 2; §13 "stitching PE not ASBR" framing in Task 12 Step 1; §17 DCI-as-migratable-unit cross-reference in Task 15 Step 2. Cross-cutting reference topology defined in Task 4 Step 3 and referenced by Tasks 6, 7, 8, 14, 15.

**Placeholder scan:** Section content briefs describe what to write rather than reproducing finished prose — same intentional adaptation as the Hero-to-Architect plan, called out at the top of "Conventions used in this plan." Each brief is concrete enough that a writer with the spec + this plan + the template guide can produce content without further clarification. No "TBD" / "TODO" / "add appropriate error handling" anywhere.

**Type consistency:** Section ids consistent throughout: `welcome`, `migration-question`, `stays-same`, `what-changes`, `ref-topology`, `header-math`, `route-types`, `walk-type2`, `walk-type5`, `walk-bum`, `hw-scale`, `coex-baseline`, `coex-services`, `gw-design`, `gw-stitching`, `gw-esi-mac`, `gw-walks`, `mig-order`, `mig-playbooks`, `final-cutover`, `troubleshooting`, `monitoring`, `decision-matrix`, `cheat-sheet`, `glossary` — total 25. CSS class names consistent: `.ph` (ported), `.pkt`/`.pkt-c`, `.cmp`/`.cmp-c`, `.stp`/`.stp-h`/`.stp-n`, `.flv-grid`/`.flv`, `.tbl`, `.info` family. Reference-topology identifiers (Leaf-1..4, Host-A/B/C/D, VNI-10100/10200, service SIDs `fc00:0:1:1:e000::` etc.) defined in Task 4 and reused consistently in Tasks 6, 7, 8, 14, 15.

**Bite-size adaptation:** For a long-form content project, strict "2–5 minute steps with full code shown" is not appropriate. Tasks are sized to one logical commit (~one section or small section group), and steps within tasks are sized to one logical content unit. The granularity here serves resumability and review — each commit produces a viewable artifact.
