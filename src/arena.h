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

int arena_add_new_heap(arena_t *a);

/* for malloc, we want to allocate from the thread-specific arena */
arena_t *arena_from_thread(void);

void ensure_global_init(void);

#endif