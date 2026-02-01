CC = gcc-15
CFLAGS = -std=c11 -Wall -Wextra -O2 -Isrc
LDLIBS = -lpthread

# Build all src/*.c automatically so newly added modules get linked.
SRCS = $(wildcard src/*.c)
SRC_OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

TEST0_BIN = build/test0
TEST1_BIN = build/test1

all: $(TEST0_BIN) $(TEST1_BIN)

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST0_BIN): tests/test0.c $(SRC_OBJS) | build
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# OpenMP build for test1 (GCC uses libgomp; no -lomp needed)
$(TEST1_BIN): tests/test1.c $(SRC_OBJS) | build
	$(CC) $(CFLAGS) -fopenmp $^ -o $@ $(LDLIBS)

run0: $(TEST0_BIN)
	./$(TEST0_BIN)

run1: $(TEST1_BIN)
	./$(TEST1_BIN)

clean:
	rm -rf build

.PHONY: all run0 run1 clean