# `smalloc`

`smalloc` is a simple memory allocator.

## Getting Started

This project is still in progress. To avoid symbol collisions for now, the API are named `my_malloc` and `my_free`. 

I’ll switch them to `malloc` and `free` soon so that the allocator can be dynamically linked into existing codebases.

```shell
make            # build test executables (requires gcc for OpenMP)
./build/test0   # run test 0 (serial)
./build/test1   # run test 1 (parallel with OpenMP)
```