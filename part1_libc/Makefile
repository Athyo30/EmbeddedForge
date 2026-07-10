# Part 1: host build. `make test` compiles your src against the harness.
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wno-unused-parameter -g -O0
SRC     := src/mem.c src/str.c src/alloc.c
TEST    := test/test_runner.c

test: $(SRC) $(TEST)
	@$(CC) $(CFLAGS) $(SRC) $(TEST) -o /tmp/forge_p1 && /tmp/forge_p1

# Address sanitizer catches your pointer bugs in the allocator early.
asan: $(SRC) $(TEST)
	@$(CC) $(CFLAGS) -fsanitize=address,undefined $(SRC) $(TEST) -o /tmp/forge_p1_asan && /tmp/forge_p1_asan

clean:
	@rm -f /tmp/forge_p1 /tmp/forge_p1_asan

.PHONY: test asan clean
