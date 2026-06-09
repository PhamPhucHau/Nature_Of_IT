##
## Master Makefile — Build and run all Java-in-C examples
##
## Usage:
##   make all       — compile everything
##   make run       — compile + run all
##   make clean     — remove all binaries
##   make one F=java_collections/01_ArrayList  — build one file
##

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11
OUTDIR  = _bin

# ── Discover all .c files ──────────────────────────────────────────────
SOURCES := $(shell find . -name '*.c' -not -path './_bin/*')
BINS    := $(patsubst ./%.c,$(OUTDIR)/%,$(SOURCES))

.PHONY: all run clean one

all: $(OUTDIR) $(BINS)
	@echo ""
	@echo "✓ Built $(words $(BINS)) executables in $(OUTDIR)/"

# Create output directory preserving subdirs
$(OUTDIR):
	mkdir -p $(OUTDIR)
	@for d in java_collections java_oop java_functional java_jvm java_modern \
	           era01_foundation era03_generics; do \
	    mkdir -p $(OUTDIR)/$$d; \
	done

# Compile rule: src/foo.c → _bin/foo
$(OUTDIR)/%.o: ./%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%: ./%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@ -lm 2>&1 | head -5; true

# Run all binaries
run: all
	@echo ""; echo "═══════════════════════════════════════"
	@echo "Running all examples"
	@echo "═══════════════════════════════════════"
	@for b in $(BINS); do \
	    echo ""; echo "▶  $$b"; echo "───────────────────────────────────────"; \
	    $$b 2>&1 | head -40; \
	done

# Build one specific file: make one F=java_collections/01_ArrayList
one:
	$(CC) $(CFLAGS) ./$(F).c -o $(OUTDIR)/$(notdir $(F)) -lm
	@echo "Built: $(OUTDIR)/$(notdir $(F))"
	$(OUTDIR)/$(notdir $(F))

clean:
	rm -rf $(OUTDIR)
	@echo "Cleaned."

# Quick individual targets
collections:
	@for f in java_collections/*.c; do \
	    out=$(OUTDIR)/java_collections/$$(basename $$f .c); \
	    mkdir -p $(OUTDIR)/java_collections; \
	    $(CC) $(CFLAGS) $$f -o $$out -lm && echo "✓ $$out" || echo "✗ $$f"; \
	done

oop:
	@for f in java_oop/*.c; do \
	    out=$(OUTDIR)/java_oop/$$(basename $$f .c); \
	    mkdir -p $(OUTDIR)/java_oop; \
	    $(CC) $(CFLAGS) $$f -o $$out -lm && echo "✓ $$out" || echo "✗ $$f"; \
	done

functional:
	@for f in java_functional/*.c; do \
	    out=$(OUTDIR)/java_functional/$$(basename $$f .c); \
	    mkdir -p $(OUTDIR)/java_functional; \
	    $(CC) $(CFLAGS) $$f -o $$out -lm && echo "✓ $$out" || echo "✗ $$f"; \
	done
