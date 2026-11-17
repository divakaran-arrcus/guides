# Memory-debugging labs. Build one lab in one variant:  make <lab>.<variant>
#   e.g.  make 06-uaf.asan && ./06-uaf.asan
#
#   plain     gcc -O0 -g -fno-omit-frame-pointer -pthread
#   asan      plain + -fsanitize=address,undefined
#   tsan      gcc -O1 -g -fsanitize=thread -pthread
#   msan      clang -O1 -g -fsanitize=memory -fsanitize-memory-track-origins -pthread
#   pattern   plain + -ftrivial-auto-var-init=pattern  (GCC fills with 0xFE bytes)
#   stripped  plain, debug info split into <lab>.debug, stripped, gnu-debuglink added
#   reftrace  plain + -DPKTD_REFTRACE -rdynamic  (boss lab only)
#
# Get a core without touching the host's core_pattern (GDB writes it):
#   make core-<lab> [V=<variant>] [ARGS=<args>]   ->  cores/<lab>.<V>[-<ARGS>].core
# It fails if the program exits normally instead of faulting.

CC    := gcc
PLAIN := -O0 -g -fno-omit-frame-pointer -pthread
HDRS  := $(wildcard pktd.h)
V     ?= plain
ARGS  ?=

%.plain: %.c $(HDRS)
	$(CC) $(PLAIN) -o $@ $<

%.asan: %.c $(HDRS)
	$(CC) $(PLAIN) -fsanitize=address,undefined -o $@ $<

%.tsan: %.c $(HDRS)
	$(CC) -O1 -g -fsanitize=thread -pthread -o $@ $<

%.msan: %.c $(HDRS)
	clang -O1 -g -fsanitize=memory -fsanitize-memory-track-origins -pthread -o $@ $<

%.pattern: %.c $(HDRS)
	$(CC) $(PLAIN) -ftrivial-auto-var-init=pattern -o $@ $<

%.stripped: %.plain
	cp $< $@
	objcopy --only-keep-debug $@ $*.debug
	strip $@
	objcopy --add-gnu-debuglink=$*.debug $@

%.reftrace: %.c $(HDRS)
	$(CC) $(PLAIN) -DPKTD_REFTRACE -rdynamic -o $@ $<

CORE = cores/$*.$(V)$(if $(ARGS),-$(ARGS)).core

core-%: %.$(V)
	@mkdir -p cores
	@rm -f $(CORE)
	-gdb -batch -ex "run $(ARGS)" -ex "generate-core-file $(CORE)" ./$<
	@test -f $(CORE) || { echo "core-$*: ./$< did not fault, no core written" >&2; exit 1; }

# Keep chained intermediates (e.g. the .plain behind core-<lab>).
.SECONDARY:

clean:
	rm -rf cores *.plain *.asan *.tsan *.msan *.pattern *.stripped *.reftrace *.debug

.PHONY: clean
