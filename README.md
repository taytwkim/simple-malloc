# Design a Memory Allocator

`malloc` is a memory allocator library for Linux that can be dynamically linked into existing codebases.

While not heavily optimized for throughput or RSS, it provides safe concurrent memory allocation through lock protection. 

Per-thread arenas and thread-local caches are used to reduce contention.

## Getting Started

1. Build `libtkmalloc.so`.

```bash
make
```

2. Compile target code, then use `LD_PRELOAD` to replace system's default `malloc` with `tkmalloc`.

```bash
# Compile target code
gcc tests/test0.c -o build/test0

# Inject tkmalloc
LD_PRELOAD=./build/libtkmalloc.so ./build/test0
```