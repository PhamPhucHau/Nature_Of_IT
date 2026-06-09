CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm

SRC     = src/task.c src/energy.c
MAIN    = src/main.c
TEST    = tests/test_phase1.c

all: phase1

phase1: $(MAIN) $(SRC)
	$(CC) $(CFLAGS) -o $@ $(MAIN) $(SRC) $(LDFLAGS)

test: $(TEST) $(SRC)
	$(CC) $(CFLAGS) -o test_runner $(TEST) $(SRC) $(LDFLAGS)
	./test_runner

run: phase1
	./phase1

debug: $(MAIN) $(SRC)
	$(CC) $(CFLAGS) -g -DDEBUG -o phase1_dbg $(MAIN) $(SRC) $(LDFLAGS)

clean:
	rm -f phase1 phase1_dbg test_runner *.o

.PHONY: all test run debug clean
