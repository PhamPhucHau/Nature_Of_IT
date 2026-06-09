CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm
SRC     = src/decision.c

all: phase4
phase4: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test: tests/test_phase4.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner $^ $(LDFLAGS) && ./test_runner
run: phase4
	./phase4
clean:
	rm -f phase4 test_runner
.PHONY: all test run clean
