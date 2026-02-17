CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Isrc -fPIC -D_GNU_SOURCE
LDLIBS = -lpthread

SRCS = src/arena.c src/freelist.c src/heap.c src/malloc.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

LIB_NAME = libsmalloc.so
LIB_PATH = build/$(LIB_NAME)

TEST0_BIN = build/test0
TEST1_BIN = build/test1
TEST2_BIN = build/test2

all: build $(LIB_PATH) $(TEST0_BIN) $(TEST1_BIN) $(TEST2_BIN)

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_PATH): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDLIBS)

$(TEST0_BIN): tests/test0.c | build
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(TEST1_BIN): tests/test1.c | build
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(TEST2_BIN): tests/test2.c | build
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS) -fopenmp

inject0: all
	LD_PRELOAD=./$(LIB_PATH) ./$(TEST0_BIN)

inject1: all
	LD_PRELOAD=./$(LIB_PATH) ./$(TEST1_BIN)

inject2: all
	LD_PRELOAD=./$(LIB_PATH) ./$(TEST2_BIN)

clean:
	rm -rf build

.PHONY: all clean inject0 inject1 inject2