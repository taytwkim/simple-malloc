#ifndef MYALLOC_TCACHE_H
#define MYALLOC_TCACHE_H

#define TCACHE_MAX_BINS 64
#define TCACHE_MAX_COUNT 32

#include "chunk.h"

typedef struct tcache_bin {
    free_chunk_t *head;   // head of the linked list
    int count;
} tcache_bin_t;

static _Thread_local tcache_bin_t g_tcache[TCACHE_MAX_BINS];  // per-thread tcache

static inline int size_to_tcache_bin(size_t usable) {
    /* Size classes are 16, 32, ...
     * 
     * Bin 0: usable in [16 .. 31]
     * Bin 1: usable in [32 .. 47]
     * ...
     */

    size_t idx = usable / 16;
    if (idx == 0) return -1;               // too small
    if (idx > TCACHE_MAX_BINS) return -1;  // too big for tcache, try free list
    return (int)(idx - 1);                 // index is 0-based
}

#endif