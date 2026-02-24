#!/bin/bash

mkdir -p build

CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O2 -Isrc -D_GNU_SOURCE"
LDLIBS="-lpthread"

echo "Building test binaries..."

$CC $CFLAGS tests/test0.c -o build/test0 $LDLIBS
echo "  [Done] build/test0"

$CC $CFLAGS tests/test1.c -o build/test1 $LDLIBS
echo "  [Done] build/test1"

$CC $CFLAGS tests/test2.c -o build/test2 $LDLIBS -fopenmp
echo "  [Done] build/test2 (OpenMP enabled)"

echo ""
echo "Compilation complete. To run with your allocator, use:"
echo "LD_PRELOAD=./build/libsmalloc.so ./build/test0"