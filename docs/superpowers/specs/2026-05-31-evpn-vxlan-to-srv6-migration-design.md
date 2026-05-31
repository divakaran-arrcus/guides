# Design Spec — EVPN-VXLAN to EVPN-SRv6 Migration Guide

**Status:** Approved skeleton + section-level design, pending user review before plan
**Author:** Divakaran Baskaran (with Claude)
**Date:** 2026-05-31
**Output artifact:** `evpn-vxlan-to-srv6-migration-guide.html` (single-page HTML, networking-guide chrome family)
**Series position:** Bridges `bgp-evpn-guide` + `dc-fabric-design-guide` (the DC overlay bucket) with `srv6-complete-guide` + `mpls-to-srv6-migration-guide` (the SRv6 bucket). Item #6 from `IDEAS.md`.

---

## 1. Purpose

A combined **comparison-and-playbook** guide for operators running EVPN-VXLAN data centers who are moving (or considering moving) the overlay data plane to SRv6. The guide answers two questions in order:

1. **What changes when you swap VXLAN encap for SRv6 encap under the same BGP-EVPN control plane?** (Part 1 + Part 2: the comparison spine.)
2. **How do you actually move a live fabric without breaking services?** (Part 3: the migration playbook — coexistence → gateway → per-service → cutover.)

## 2. Audience

Primary: network operators with a live EVPN-VXLAN fabric who need to migrate to SRv6 — coexistence, gateway stitching, per-service cutover, brownfield safety.
Secondary: architects choosing between VXLAN and SRv6 for a new DC, who need the head-to-head comparison.

Assumes the reader has working knowledge of: BGP-EVPN with VXLAN (covered in `bgp-evpn-guide`), leaf-spine DC fabrics (`dc-fabric-design-guide`), and SRv6 fundamentals incl. uSID (`srv6-complete-guide`, optionally `srv6-network-programming-guide`).

## 3. Non-goals

- Not an EVPN tutorial — control-plane fundamentals are in `bgp-evpn-guide`, not re-taught here.
- Not a DC fabric design guide — leaf-spine, ECMP, BGP underlay are in `dc-fabric-design-guide`.
- Not an SRv6 tutorial — SID structure, micro-SIDs, behaviors are in `srv6-complete-guide`.
- Not a vendor configuration manual — examples are vendor-neutral pseudocode.
- Not an MPLS-EVPN migration guide — that's a different starting state (covered partially in `mpls-to-srv6-migration-guide`).
- No SR-MPLS-EVPN coverage (different data plane; flagged in §11 if relevant, not a focus).

## 4. Positioning vs existing guides

| | This guide | bgp-evpn | dc-fabric | srv6-complete | mpls-to-srv6-migration |
|---|---|---|---|---|---|
| Topic | VXLAN→SRv6 migration of EVPN | EVPN over VXLAN | Leaf-spine + overlay | SRv6 fundamentals + EVPN brief | MPLS→SRv6 migration |
| EVPN treatment | Both encaps, side-by-side + playbook | VXLAN encap only | VXLAN encap (overlay context) | One section (EVPN with SRv6) | One section (EVPN-SRv6 migration) |
| Reader's starting state | Has EVPN-VXLAN running | Learning EVPN | Designing a fabric | Learning SRv6 | Has MPLS running |
| Output | Migration playbook + comparison | Reference | Reference | Reference | Migration playbook |

The new guide is genuinely net-new: no existing guide treats VXLAN→SRv6 migration as the main subject. The closest overlap is `mpls-to-srv6-migration-guide` (3 short EVPN sections, total ~17 mentions), which this guide complements rather than duplicates.

## 5. Framing rules (apply throughout)

1. **All body prose is point-wise from the start.** Each `<h2>` section has a short prose lede setting context, then `<h4 class="ph">` mini-headings with bullet lists for everything else. No dense paragraphs.
2. **Visual components carry weight wherever they shine:** `.pkt` byte-box diagrams for headers and packet walks, `.cmp` two-column for VXLAN-vs-SRv6 side-by-sides, `.stp` for sequential flows (migration steps, packet hops, debug procedures), `.flv-grid` for sets (route types, service callouts, design options), `.info`/`.info.green`/`.info.orange`/`.info.red`/`.info.yellow` callouts for guidance, gotchas, and warnings.
3. **Phase-organized playbook (hybrid).** Top-level structure is the four migration phases (coexistence → gateway → per-service → cutover). The gateway phase gets extra depth (4 of 9 playbook sections). Service callouts appear *inside* each phase so a service-targeted reader can navigate by service too.
4. **uSID-primary for SRv6 examples.** Modern SRv6 deployments use uSID (compressed SRv6). Classic SRv6 SRH treatment appears as a variant where relevant, not as the default.
5. **Vendor-neutral pseudocode** for any CLI examples; consistent with the rest of the series.

## 6. Visual style

Match the existing networking-guide chrome family (`bgp-evpn`, `dc-fabric`, `srv6-complete`):
- Sidebar navigation (left, fixed, flat h2 outline grouped by Part)
- Dark/light mode toggle
- Existing CSS components: `.pkt`/`.pkt-c` (byte boxes with `bg-ipv`, `bg-mpls`, `bg-seg`, `bg-srh`, `bg-pay`, `bg-vpn`, etc.), `.cmp`/`.cmp-c`, `.stp`/`.stp-h`/`.stp-n` (color variants `grn`/`org`/`red`/`pur`), `.flv-grid`/`.flv`, `.tbl`, `.info` family, `.ann`, `.card`, `.sec-title`/`.sec-sub`, `<dl class="glossary">`
- The `.ph` mini-heading class (from `gen-ai-hero-to-architect-guide.html`) plus its block-layout + inline-`<code>` CSS — needed because *all* body prose in this guide is point-wise from the start.

## 7. Structure — 25 sections, flat h2 spine

### Front matter (2 sections)
- **§FM-1 Welcome** — who this is for, prerequisites, how to read (textbook vs reference).
- **§FM-2 The migration question** — motivation: header tax, scale, SP+DC underlay unification, service insertion, vendor-driven moves.

### Part 1 — Foundations (3 sections)
- **§1. What stays the same — BGP-EVPN control plane.** AFI/SAFI, route types, MAC/IP learning, EVPN config shape. `.flv-grid` 4 cards.
- **§2. What changes — encap, endpoint, service ID.** VXLAN→SRv6 header swap, VTEP→SRv6 endpoint, VNI→service SID. uSID note. `.cmp` two-column + `.pkt` stacked encap diagrams.
- **§3. Reference topology.** 2-spine / 4-leaf, two VLANs+VNIs, one VRF (IRB), 3 topology states (A=VXLAN, B=SRv6, M=mid-migration). Inline topology diagram + `.flv-grid` (3 state cards).

### Part 2 — Comparison (6 sections)
- **§4. Header math.** VXLAN stack vs SRv6 stack bytes; header-tax table. `.pkt` byte-box diagrams (3 stacks) + `.tbl` matrix.
- **§5. Route-type behavior across encaps.** Type 2/3/4/5 per-encap attributes. `.cmp` or 4-row `.tbl` + `.info` callout on next-hop encoding.
- **§6. Packet walk 1 — Type 2 MAC/IP, side-by-side.** `.cmp` two-column with `.stp` + `.pkt` per side. Foundational walk.
- **§7. Packet walk 2 — Type 5 IP Prefix, side-by-side.** `.cmp` two-column with `.stp` + `.pkt`. `.info.green` "safe first cut."
- **§8. Packet walk 3 — Type 3 IMET / BUM.** Topology flood diagram + per-encap `.pkt`. `.info.yellow` on PIM underlay.
- **§9. Hardware, silicon, scale, MTU, operational tooling.** `.tbl` dimension × encap matrix + `.flv-grid` of sharpest deltas.

### Part 3 — Migration Playbook (9 sections — hybrid: phase spine + gateway depth + service callouts)

**Phase 1 · Coexistence (2)**
- **§10. Coexistence baseline — dual-stack fabric design.** Topology State M + `.cmp` BGP attributes + `.info.yellow` next-hop gotcha.
- **§11. Coexistence — per-service callouts.** `.flv-grid` 5 cards (L2, L3, IRB, multi-homing, DCI) + `.info` "pick one service to lead."

**Phase 2 · Gateway / border node (4 — the depth cluster)**
- **§12. Gateway design.** Border-leaf vs spine-as-gateway vs dedicated DCI. `.cmp` three-column + `.tbl` decision matrix + inline topology diagrams.
- **§13. Route-type stitching mechanics.** Gateway as EVPN ASBR. `.stp` generic stitching pipeline + `.tbl` per-route-type translation + `.info.orange` BGP session callout.
- **§14. ESI + MAC mobility + ARP/ND across encaps.** Operational hazards. `.stp` MAC mobility traversal + `.info.red` 3 pitfalls.
- **§15. Gateway packet walks — VXLAN host → Gateway → SRv6 host (forward + return).** `.cmp` two-column with `.stp` + `.pkt` per direction + topology diagram. **Packet walks 4–5.**

**Phase 3 · Per-service migration (2)**
- **§16. Migration order and approach.** Recommended order (L3 → IRB → L2 single-homed → L2 multi-homed → DCI). Cutover patterns (per-VRF / per-VLAN / per-tenant / per-leaf-group). Rollback design. `.stp` 5-step order + `.flv-grid` 4 patterns + `.info.green` "reversible without gateway reconfig."
- **§17. Service-specific cutover playbooks.** One `<h4 class="ph">` per service with pre-check → cutover → validation → rollback. `.flv-grid` 5-card index + `.tbl` per service.

**Phase 4 · Cutover (1)**
- **§18. Final cutover, rollback, cleanup.** Retire the gateway, VXLAN-specific cleanup, post-migration validation. `.stp` decom sequence + `.info.orange` rollback-after-cleanup warning.

### Part 4 — Operational (2 sections)
- **§19. Troubleshooting — control plane vs data plane.** `.cmp` debug toolkit per encap + `.tbl` failure×symptom×fix + `.info.red` MTU classic.
- **§20. Monitoring, telemetry, observability.** Counter mapping, flow export, SLA probes, dashboards during dual-stack. `.tbl` monitoring matrix.

### Reference (3 sections)
- **§21. Decision matrix — migrate now / later / stay.** `.tbl` signal×outcome + `.flv-grid` 3 outcomes.
- **§22. VXLAN vs SRv6 cheat sheet.** Dense `.tbl` reference — encap headers, show commands, route attrs, capture filters, common SID functions.
- **§23. Glossary delta.** `<dl class="glossary">` of terms specific to this guide: Encapsulation Ext Community, ES Label/SID, End.DT2U/2M/4/6, Gateway/border-leaf, IMET, PMSI Tunnel attr, Re-origination, Service SID, SRv6 SID Tunnel attribute, Stitching, uN, uA, VNI/Service SID mapping, VTEP/SRv6 endpoint equivalence.

## 8. Sections-and-walks summary

| Part | Sections | h2 count |
|------|----------|---------|
| Front matter | Welcome, Migration question | 2 |
| Part 1 Foundations | Stays · Changes · Topology | 3 |
| Part 2 Comparison | Header math · Route types · 3 packet walks · HW/scale/MTU | 6 |
| Part 3 Playbook | Coexistence (2) · Gateway (4) · Per-service (2) · Cutover (1) | 9 |
| Part 4 Operational | Troubleshooting · Monitoring | 2 |
| Reference | Decision matrix · Cheat sheet · Glossary | 3 |
| **Total** | | **25** |

**Total packet walks:** 5 (Type 2 in 2 encaps + Type 5 in 2 encaps + Type 3 BUM + gateway forward+return as one section with two walks inside).

## 9. Cross-cutting threads

1. **Reference topology threads through every walk and migration scenario.** Defined in §3 (states A/B/M), referenced by §6/§7/§8/§15.
2. **Service callouts** appear in two places: light-touch index callouts inside Phase 1 (§11) and per-service deep playbooks in Phase 3 (§17). Cross-linked.
3. **Point-wise format throughout** — every body prose section is `.ph` mini-headings + bullets after a short prose lede. No dense paragraphs.
4. **Side-by-side comparison pattern** (`.cmp` two-column) repeats in §2, §4, §6, §7, §9, §12, §15, §19 — a consistent visual cue for "VXLAN-side / SRv6-side."

## 10. Out-of-scope deferrals (candidates for future guides)

- SR-MPLS-EVPN migration to SRv6-EVPN (different starting state; partially covered in `mpls-to-srv6-migration-guide`).
- Network automation for the migration (Ansible/Terraform/NETCONF orchestration of the cutover) — out of scope; would be its own guide (IDEAS.md #4).
- Vendor-specific configuration syntax — vendor-neutral pseudocode only.
- Detailed PIM-multicast underlay migration for VXLAN deployments that used native multicast — flagged in §8 but not deep-treated.

## 11. Implementation outline (for the writing-plans skill)

1. **HTML scaffold** — copy a networking-guide template (likely `mpls-to-srv6-migration-guide.html` since it's the closest playbook structure); update title, sidebar brand, intro; strip existing content; preserve all CSS components.
2. **CSS additions** — port the `.ph` mini-heading rule from `gen-ai-hero-to-architect-guide.html` (with the block layout + inline `<code>` support already finalized there).
3. **Sidebar navigation** — flat h2 list with Part-label dividers (Front matter / Part 1 / Part 2 / Part 3 / Part 4 / Reference).
4. **Section-by-section content** — write in order (§FM-1 through §23). Use existing networking-guide voice; point-wise throughout per §5 framing rules.
5. **Packet walks** — 5 total; use `.pkt` byte-box diagrams + `.stp` step-flows + `.cmp` two-column wrappers for side-by-side comparisons.
6. **Reference topology** — define once in §3; reuse identifiers (leaf-1, leaf-3, host-A, host-B, VNI-10100, service SIDs, etc.) consistently across all walks.
7. **Cross-link verification** — every `onclick="go(...)"` must hit a real section ID; orphan-check pass before final commit.
8. **Visual review pass** — render in browser, dark/light toggle, narrow-width responsiveness, zero console errors.
9. **README update** — add a row to the "SRv6 Advanced & Migration" bucket.
10. **IDEAS.md update** — mark item #6 as Done.

## 12. Success criteria

- An operator running an EVPN-VXLAN DC can read this guide and end up with a credible migration plan: scheduling phases, gateway design, per-service cutover order, rollback considerations.
- An architect can pull §4 / §5 / §9 / §21 and walk into a decision meeting with the head-to-head comparison answered.
- A reader who finished `bgp-evpn-guide` + `dc-fabric-design-guide` + `srv6-complete-guide` finds nothing redundant here.
- All 5 packet walks render correctly and a reader can trace a packet end-to-end without ambiguity.
- The point-wise format keeps the prose scannable; no `<p>` longer than 2 sentences (other than chapter-opening ledes).
- Visual integrity matches the rest of the networking-guide family.

## 13. Open items at spec-writing time

- **Title** — current working title is "EVPN-VXLAN to EVPN-SRv6 Migration Guide." Could be tightened to "EVPN: VXLAN to SRv6 Migration" or similar. Final pick during HTML scaffold.
- **Accent color** — the guide is in the SRv6 family; could share the SRv6 accent (cyan/teal) or introduce a slight differentiator. Decide during scaffold.
- **Reference-topology address conventions** — exact loopback IPs and uN locator block to be pinned in §3 during writing (currently described in shape only).
- **Gateway placement default in examples** — border-leaf is the most common pattern; will be the default in all walks unless §12 narrative demands otherwise.
