#include "freelist.h"

void free_list_remove(arena_t *a, free_chunk_t *fc) {
    free_chunk_t *fd = fc->prev, *bk = fc->next;
    if (bk) bk->prev = fd;
    if (fd) fd->next = bk;
    if (a->free_list == fc) a->free_list = fd;
    fc->prev = fc->next = NULL;
}

void free_list_push_front(arena_t *a, free_chunk_t *fc) {
    fc->next = NULL;
    fc->prev = a->free_list;
    if (a->free_list) a->free_list->next = fc;
    a->free_list = fc;
}

void* free_list_try(arena_t *a, size_t need_total) {
    for (free_chunk_t *p = a->free_list; p; p = p->prev) {
        if (!chunk_is_free(p)) continue;
        if (chunk_get_size(p) >= need_total) {
            heap_t *h = chunk_get_heap(p);
            return heap_split_free_chunk(h, p, need_total);
        }
    }
    return NULL;
}