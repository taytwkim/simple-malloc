CC = gcc
# -fPIC is essential for shared libraries
CFLAGS = -std=c11 -Wall -Wextra -O2 -Isrc -fPIC -D_GNU_SOURCE
LDLIBS = -lpthread

# Library source files
SRCS = src/arena.c src/freelist.c src/heap.c src/malloc.c src/config.c
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

LIB_NAME = libsmalloc.so
LIB_PATH = build/$(LIB_NAME)

all: build $(LIB_PATH)

build:
	mkdir -p build

# Compiles object files with Position Independent Code (-fPIC)
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Links object files into the shared library
$(LIB_PATH): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -rf build

.PHONY: all clean