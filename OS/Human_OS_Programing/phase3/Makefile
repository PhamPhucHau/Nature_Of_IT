CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm
SRC     = src/energy_model.c

all: phase3
phase3: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test: tests/test_phase3.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner $^ $(LDFLAGS) && ./test_runner
run: phase3
	./phase3
clean:
	rm -f phase3 test_runner
.PHONY: all test run clean
