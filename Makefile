CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -fPIC -D_GNU_SOURCE
LDLIBS = -lpthread

SRCS = src/arena.c src/freelist.c src/heap.c src/malloc.c src/config.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

LIB_NAME = libtkmalloc.so
LIB_PATH = build/$(LIB_NAME)

all: build $(LIB_PATH)

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_PATH): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -rf build

.PHONY: all clean