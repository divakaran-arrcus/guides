# Guide Ideas

Future guides to build. The **Up next** table is in build order; the shipped table is the record.

## Up next

| Order | Guide | Audience Bridge | Status |
|-------|-------|-----------------|--------|
| 1 | **SRv6 Service Programming with uSID** | Old #9 (SFC). uSID only — no G-SID / REPLACE-C-SID. SR-aware (End.AN) vs SR-unaware proxies (End.AS / End.AD / End.AM) in uSID networks, stateful chains, chains across domains. Links to `srv6-network-programming-guide.html` for uSID basics instead of repeating them | Planned |
| 2 | **IS-IS from First Principles** | The IGP under every SR / SRv6 guide — adjacency FSM, LSP flooding and purging, SPF, levels and route leaking, SR / SRv6 TLVs. Lab kit with open-source routing containers | Planned |
| 3 | **Fast Convergence End to End** | BFD, SPF / LSP throttling, prefix prioritization, BGP PIC core / edge, FIB download time; ties together the existing TI-LFA material | Planned |
| 4 | **Network Automation with BGP** | Provisioning EVPN / SRv6 VPNs from code — gNMI / YANG, intent checks | Planned |

## Shipped

| Guide | File | Notes |
|-------|------|-------|
| MPLS to SRv6 Migration | `mpls-to-srv6-migration-guide.html` | Coexistence, interworking, cutover |
| SR-MPLS Complete Guide | `sr-mpls-guide.html` | Prequel for LDP / RSVP-TE engineers |
| Data Center Fabric Design | `dc-fabric-design-guide.html` | Spine-leaf, underlay, overlay choices |
| Segment Routing Policy (SR-TE) | `sr-te-policy-guide.html` | ODN, PCE, constraints, optimization |
| EVPN-VXLAN to EVPN-SRv6 Migration | `evpn-vxlan-to-srv6-migration-guide.html` | 2026-06-01 — comparison, 4-phase playbook, 5 packet walks |
| SRv6 Network Programming | `srv6-network-programming-guide.html` | 2026-03-14 — uSID ops, TE policies, binding SIDs, cross-domain, packet walks |
| SRv6 TI-LFA & Fast Reroute | — | Audit-and-fill, not standalone: gaps filled in `srv6-complete`, `sr-mpls` and `sr-te-policy` |
| Gen-AI: Zero to Hero | `gen-ai-zero-to-hero-guide.html` | 2026-05-26 |
| Gen-AI: Hero to Architect | `gen-ai-hero-to-architect-guide.html` | 2026-05-27 |
| Gen-AI: Architect to Leader | `gen-ai-architect-to-leader-guide.html` | 2026-06-01 |
| Gen-AI: Harness Engineering | `gen-ai-harness-engineering-guide.html` | 2026-11-02 — Part 4 |
| Memory Debugging from First Principles | `memory-debugging-guide.html` | 2026-11-17 — with `labs/memory-debugging/` kit |
| Performance & Tracing for Network Daemons | `perf-tracing-guide.html` | 2026-11-19 — sequel to Memory Debugging, with `labs/perf-tracing/` kit |
| Chess for Kids | `chess-for-kids-guide.html` | 2026-06-13 |
