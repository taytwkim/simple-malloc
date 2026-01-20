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

void arena_init(arena_t *a, int id);

arena_t *get_my_arena(void);

void arena_set_next_chunk_prev_in_use(arena_t *a, void *hdr, int prev_in_use);

void set_next_chunk_hdr_prev(arena_t *a, void *hdr, int prev_in_use);

void* split_free_chunk(arena_t *a, free_chunk_t *fc, size_t need_total);

void* carve_from_top(arena_t *a, size_t need_total);

void* coalesce(arena_t *a, void *hdr);

#endif
