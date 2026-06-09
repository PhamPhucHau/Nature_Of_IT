CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm
SRC     = src/learning.c

all: phase5
phase5: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test: tests/test_phase5.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner $^ $(LDFLAGS) && ./test_runner
run: phase5
	./phase5
clean:
	rm -f phase5 test_runner
.PHONY: all test run clean
