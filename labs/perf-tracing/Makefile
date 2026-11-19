# Performance & tracing labs. Build one lab:  make 01-hot-path
# Every lab is one compile line; the fixed variant is a run-time flag (--fixed, or --fix=... for
# the boss level), so before and after are the same binary.
#
#   -O2                        optimised, like a real daemon
#   -g                         symbols and line numbers for perf report / bpftrace
#   -fno-omit-frame-pointer    so perf and bpftrace can walk stacks with frame pointers
#                              (leaf functions still get no frame: see the guide, rung 1)
#   -fno-optimize-sibling-calls    keep every call a call, so no caller vanishes from a stack
#
# The pipeline labs include pipeline.h, which uses <sys/sdt.h> for the USDT probes.

.DEFAULT_GOAL := all

CC     := gcc
CFLAGS := -O2 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -pthread -Wall
LABS   := f1-where-time-goes 00-baseline 01-hot-path 02-lock 03-syscalls 04-sched 05-throttle 06-boss

all: $(LABS)

%: %.c pipeline.h Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(LABS) perf.data perf.data.old

.PHONY: all clean
