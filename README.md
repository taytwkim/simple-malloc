# smalloc

`smalloc` is a simple memory allocator for Linux.

## Getting Started

1. Build `libsmalloc.so`.

```bash
make
```

2. Compile your code, then use `LD_PRELOAD` to replace default `malloc` with `smalloc`.

```bash
# Compile your code
gcc tests/test0.c -o build/test0

# Inject smalloc
LD_PRELOAD=./build/libsmalloc.so ./build/test0
```