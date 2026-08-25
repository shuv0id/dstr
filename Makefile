CFLAGS = -Wall -Wextra -Werror -pedantic -std=c17 -fsanitize=address,undefined
CC = gcc
TEST_BIN_DIR = ./test-bin

.PHONY: all format test clean

all: test

format: dstr.h test_dstr.c
	clang-format -i $^

test_runner: test_dstr.c
	mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $(TEST_BIN_DIR)/$@ $^

test: test_runner
	$(TEST_BIN_DIR)/$^

clean:
	rm -rf $(TEST_BIN_DIR)
