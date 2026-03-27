<h1 align="center">Design a Memory Allocator</h1>

`tkmalloc` is a memory allocator for Linux that can be dynamically linked into existing codebases.

While not heavily optimized for throughput or RSS, it provides safe concurrent memory allocation through lock protection. 
Per-thread arenas and thread-local caches are used to reduce contention.

## Getting Started

### On Linux

1. Build `libtkmalloc.so`.

```shell
make
```

2. Compile target code, then use `LD_PRELOAD` to replace system's default `malloc` with `tkmalloc`.

```shell
# Compile target code
gcc tests/test0.c -o build/test0

# Inject tkmalloc
LD_PRELOAD=./build/libtkmalloc.so ./build/test0
```

### On macOS

`tkmalloc` does not currently support macOS. To run quick tests from macOS, use the Docker script with one of the test files under the `tests/` directory.

```shell
./scripts/docker_run.sh tests/test0.c
```
