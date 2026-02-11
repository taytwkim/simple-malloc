#ifndef MYALLOC_ARENA_H
#define MYALLOC_ARENA_H

#include <pthread.h>
#include "chunk.h"
#include "heap.h"

#define MAX_NUM_ARENAS 64

typedef struct arena {
    int id;
    heap_t *heaps;
    heap_t *active_heap; // for now, let's assume that the active_heap is always the heap that was last added
    free_chunk_t *free_list;
    pthread_mutex_t lock;
} arena_t;

/* for malloc, we want to allocate from the thread-specific arena */
arena_t *arena_from_thread(void);

/* 
 * TO DO: for free, we want to find the right "heap" to return
 * Note that freelist functions need the arena, but finding the arena is easy if you find the heap (we just do h->arena)
 * This function will eventually be removed, it's logic is wrong now anyway.
 */
arena_t *arena_from_chunk_header(void *hdr);

void ensure_global_init(void);

#endif