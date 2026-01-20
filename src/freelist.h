#ifndef MYALLOC_FREELIST_H
#define MYALLOC_FREELIST_H

#include "arena.h"

void remove_from_free_list(arena_t *a, free_chunk_t *fc);

void push_front_to_free_list(arena_t *a, free_chunk_t *fc);

void* try_free_list(arena_t *a, size_t need_total);

#endif