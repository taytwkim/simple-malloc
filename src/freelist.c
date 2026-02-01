#include "freelist.h"

void free_list_remove(arena_t *a, free_chunk_t *fc) {
    free_chunk_t *fd = fc->fd, *bk = fc->bk;
    if (bk) bk->fd = fd;
    if (fd) fd->bk = bk;
    if (a->free_list == fc) a->free_list = fd;
    fc->fd = fc->bk = NULL;
}

void free_list_push_front(arena_t *a, free_chunk_t *fc) {
    fc->bk = NULL;
    fc->fd = a->free_list;
    if (a->free_list) a->free_list->bk = fc;
    a->free_list = fc;
}

void* free_list_try(arena_t *a, size_t need_total) {
    for (free_chunk_t *p = a->free_list; p; p = p->fd) {
        if (!chunk_is_free(p)) continue;
        if (chunk_get_size(p) >= need_total) return heap_split_free_chunk(a, p, need_total);
    }
    return NULL;
}