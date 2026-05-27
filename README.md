# Technical Guides

A collection of interactive, single-page HTML guides for networking, infrastructure, and AI engineering topics.

Each guide is a self-contained HTML file with sidebar navigation, dark/light mode toggle, and rich visual components (packet diagrams, comparison grids, tables, info boxes, and more).

## Guides

### SR-MPLS & SRv6 Core

Start here for Segment Routing fundamentals and the SRv6 data plane.

| Guide | File | Description |
|-------|------|-------------|
| SR-MPLS Complete Guide | [sr-mpls-guide.html](sr-mpls-guide.html) | From LDP/RSVP-TE to Segment Routing — SRGB/SRLB, Prefix-SID, Adj-SID, ISIS/OSPF extensions, TI-LFA, SR-Policy, Flex-Algo, BGP-LS, L3VPN, and LDP migration |
| SRv6 Complete Guide | [srv6-complete-guide.html](srv6-complete-guide.html) | SRv6 for SR-MPLS engineers — covers SID structure, micro-SIDs, behaviors, flavors, TI-LFA, SR-Policy, VPN, and full packet walks |
| SR-TE Policy Guide | [sr-te-policy-guide.html](sr-te-policy-guide.html) | Segment Routing Traffic Engineering — SR Policies, ODN, PCE/PCEP, constraints, flex-algo integration, BSID chaining, VPN steering, and full packet walks |

### SRv6 Advanced & Migration

Deeper SRv6 operations and moving an existing MPLS network onto SRv6.

| Guide | File | Description |
|-------|------|-------------|
| SRv6 Network Programming | [srv6-network-programming-guide.html](srv6-network-programming-guide.html) | Advanced SRv6 operations — uSID behaviors (uN, uA, uDT, uDX, uB6), headend encapsulation, SRv6 TE policies, cross-domain B6 chaining, SR-MPLS interworking, and 5 detailed packet walks |
| MPLS to SRv6 Migration Guide | [mpls-to-srv6-migration-guide.html](mpls-to-srv6-migration-guide.html) | SR-MPLS to SRv6 migration — coexistence models, gateway stitching, per-service migration (L3VPN, L2VPN, SR-TE), phase planning, and full packet walks |

### Data Center & EVPN

Leaf-spine fabrics and the BGP EVPN control plane.

| Guide | File | Description |
|-------|------|-------------|
| BGP EVPN Guide | [bgp-evpn-guide.html](bgp-evpn-guide.html) | From VXLAN fundamentals to production L2VPN — VLANs, VXLAN packet format, BGP EVPN route types, ARP suppression, multi-homing, IRB, and full deployment walkthrough |
| DC Fabric Design Guide | [dc-fabric-design-guide.html](dc-fabric-design-guide.html) | From legacy 3-tier to modern leaf-spine — IP fabrics, VXLAN, BGP EVPN, multi-tenancy, symmetric IRB, multi-site DCI, and full packet walks |

### Gen AI

From AI fundamentals to the modern agentic coding stack.

| Guide | File | Description |
|-------|------|-------------|
| Gen AI: Zero to Hero | [gen-ai-zero-to-hero-guide.html](gen-ai-zero-to-hero-guide.html) | Complete guide from AI basics to mastery — neural networks, LLMs, transformers, prompting, RAG, AI agents, Claude, Claude Code, CLAUDE.md, skills, MCP, hands-on exercises, cheat sheets, and A-Z glossary |
| Gen AI: Hero to Architect | [gen-ai-hero-to-architect-guide.html](gen-ai-hero-to-architect-guide.html) | Advanced companion to Zero to Hero — agentic coding loop, context engineering, OpenSpec, SpecKit, Superpowers, Kilo Code, Claude Code, Gemini CLI, MCP, multi-agent workflows, and a worked example shipping a CLI tool across three tools |

## Usage

Open any `.html` file directly in a browser. No build step or server required.
