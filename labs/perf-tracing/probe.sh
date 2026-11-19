#!/bin/bash
# probe.sh — which perf / eBPF capabilities work in this container on this host?
# Prints one "PROBE <name> ok|fail" line per capability. Every probe is scoped to its own
# child process (no system-wide perf, no unfiltered tracepoints) and runs under a timeout.
set -u
cd "$(dirname "$0")" || exit 2
T="timeout -k 2 20"
work=/tmp/probe-spin
printf 'int main(void){volatile unsigned long i=0;for(;i<300000000UL;i++);return 0;}\n' > /tmp/probe-spin.c
gcc -O1 -o $work /tmp/probe-spin.c || exit 2

p() { # name, command...
  local name=$1; shift
  if $T bash -c "$*" >/tmp/probe.$name 2>&1; then echo "PROBE $name ok"; else echo "PROBE $name fail"; sed 's/^/    /' /tmp/probe.$name | tail -3; fi
}

p perf-stat-hw      "perf stat -e cycles,instructions -x, -- $work 2>&1 | grep -q '^[0-9]*,,cycles'"
p perf-stat-sw      "perf stat -e task-clock,context-switches -- $work"
p perf-record-g     "perf record -q -g -o /tmp/p.data -- $work && perf report -i /tmp/p.data --stdio 2>/dev/null | grep -q main"
p perf-sched-tp     "perf record -q -e sched:sched_switch -o /tmp/s.data -- sleep 0.1 && perf script -i /tmp/s.data | grep -q sched_switch"
p perf-sched-timehist "perf record -q -e sched:sched_switch,sched:sched_wakeup,sched:sched_waking,sched:sched_stat_runtime -o /tmp/t.data -- $work && perf sched timehist -i /tmp/t.data 2>&1 | grep -q probe-spin"
p perf-trace        "perf trace -s -- sleep 0.1 2>&1 | grep -q nanosleep"
p perf-lock-bpf     "perf lock contention -b -- sleep 0.1"
p bpftrace-uprobe   "bpftrace -e 'uprobe:$work:main { printf(\"hit\\n\"); exit(); }' -c $work | grep -q hit"
p bpftrace-tp-comm  "bpftrace -e 'tracepoint:syscalls:sys_enter_clock_nanosleep /comm == \"sleep\"/ { printf(\"hit\\n\"); exit(); }' -c '/bin/sleep 0.2' | grep -q hit"
p bpftrace-sched    "bpftrace -e 'tracepoint:sched:sched_switch /args->prev_comm == \"probe-spin\"/ { @[args->prev_state] = count(); }' -c $work"
echo "kernel $(uname -r); perf $(perf version 2>&1 | head -1); $(bpftrace --version)"
