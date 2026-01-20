#include "freelist.h"

void remove_from_free_list(arena_t *a, free_chunk_t *fc) {
    free_chunk_t *fd = fc->links.fd, *bk = fc->links.bk;
    if (bk) bk->links.fd = fd;
    if (fd) fd->links.bk = bk;
    if (a->free_list == fc) a->free_list = fd;
    fc->links.fd = fc->links.bk = NULL;
}

void push_front_to_free_list(arena_t *a, free_chunk_t *fc) {
    fc->links.bk = NULL;
    fc->links.fd = a->free_list;
    if (a->free_list) a->free_list->links.bk = fc;
    a->free_list = fc;
}

void* try_free_list(arena_t *a, size_t need_total) {
    for (free_chunk_t *p = a->free_list; p; p = p->links.fd) {
        if (!chunk_is_free(p)) continue;

        if (get_chunk_size(p) >= need_total) return split_free_chunk(a, p, need_total);
    }
    return NULL;
}