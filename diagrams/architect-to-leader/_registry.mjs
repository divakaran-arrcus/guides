export const REGISTRY = {
  'maturity-model':    { kind: 'mermaid',  title: 'The five-stage maturity model: Ad-hoc to Enterprise' },
  'config-layering':   { kind: 'mermaid',  title: 'Golden config layering: org baseline to effective config' },
  'cicd-pipeline':     { kind: 'mermaid',  title: 'Agents in CI/CD: deterministic gates decide, the agent advises' },
  'cost-by-team':      { kind: 'barChart', data: 'cost-by-team.chart.json', title: 'FinOps showback: illustrative monthly spend by team' },
  'threat-surface':    { kind: 'mermaid',  title: 'The expanded threat surface when agents act, and its controls' },
  'incident-response': { kind: 'mermaid',  title: 'Incident response: detect, stop, contain, roll back, postmortem, harden' },
  'paved-road':        { kind: 'mermaid',  title: 'The paved-road model: mandate, recommend, leave free' },
  'rollout-timeline':  { kind: 'timeline', data: 'rollout-timeline.svg.json', title: 'A 12-month rollout across the maturity stages' },
};
