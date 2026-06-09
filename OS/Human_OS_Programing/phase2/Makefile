CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm
SRC     = src/task_queue.c src/scheduler.c

all: phase2
phase2: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test: tests/test_phase2.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner $^ $(LDFLAGS) && ./test_runner
run: phase2
	./phase2
clean:
	rm -f phase2 test_runner
.PHONY: all test run clean
