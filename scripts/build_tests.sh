#!/bin/bash

mkdir -p build

CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O2 -Isrc -D_GNU_SOURCE"
LDLIBS="-lpthread"

echo "Building test binaries..."

$CC $CFLAGS tests/hello.c -o build/hello $LDLIBS
echo "  [Done] build/hello"

$CC $CFLAGS tests/sequential.c -o build/sequential $LDLIBS
echo "  [Done] build/sequential"

$CC $CFLAGS tests/parallel.c -o build/parallel $LDLIBS -fopenmp
echo "  [Done] build/parallel (OpenMP enabled)"

echo ""
echo "Compilation complete. To run with your allocator, use:"
echo "LD_PRELOAD=./build/libtkmalloc.so ./build/hello"

echo ""
echo "To check that the symbol has been injected, set TKMALLOC_INJECTED environment variable:"
echo "TKMALLOC_INJECTED=1 LD_PRELOAD=./build/libtkmalloc.so ./build/hello"

echo ""
echo "To enable logging, set TKMALLOC_VERBOSE environment variable:"
echo "TKMALLOC_VERBOSE=1 LD_PRELOAD=./build/libtkmalloc.so ./build/hello"

echo ""
