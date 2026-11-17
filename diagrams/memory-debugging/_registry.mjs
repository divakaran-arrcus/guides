export const REGISTRY = {
  'capture-pipeline':       { kind: 'mermaid', title: 'How a lab run becomes a terminal block: build, capture, check, sync' },
  'path-ab-flow':           { kind: 'mermaid', title: 'Path A (it reproduces) versus Path B (only a core), and how a reproducer joins them' },
  'instrumentation-models': { kind: 'mermaid', title: 'Three instrumentation models: compile-time checks, dynamic binary translation, record and replay' },
  'happens-before':         { kind: 'mermaid', title: 'Unlock then lock of the same mutex makes the write happen-before the read' },
  'lock-cycle':             { kind: 'mermaid', title: 'A lock-order cycle: each thread holds one lock and waits for the other' },
  'boss-timeline':          { kind: 'mermaid', title: 'Boss level timeline: a put with no get frees session 7 while the table still points to it' },
  'core-to-repro':          { kind: 'mermaid', title: 'From a one-shot core to a hypothesis, a harness, and an rr recording' },
};
