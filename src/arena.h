#ifndef MYALLOC_ARENA_H
#define MYALLOC_ARENA_H

#include <pthread.h>
#include <stdint.h>
#include "chunk.h"

#define MAX_NUM_ARENAS 64

typedef struct arena {
    int id;
    uint8_t *base;
    uint8_t *bump;
    uint8_t *end;
    free_chunk_t *free_list;
    pthread_mutex_t lock;
} arena_t;

/* returns offset compared to arena base */
static inline int OFF(arena_t *a, void *p) {
    return (int)((uintptr_t)p - (uintptr_t)a->base);
}

void arena_init(arena_t *a, int id);

arena_t *get_my_arena(void);

void arena_set_next_chunk_P(arena_t *a, void *hdr, int P);

/* if the freelist does not have a suitable chunk, carve from the top */
void* arena_carve_from_top(arena_t *a, size_t need_total);

/* merge chunk with adjacent free chunks (adjacent in memory, not in the linked list) */
void* arena_coalesce_free_chunk(arena_t *a, void *hdr);

/* if the free chunk is large enough, split the chunk */
void* arena_split_free_chunk(arena_t *a, free_chunk_t *fc, size_t need);

#endif