#include "arena.h"
#include "freelist.h"
#include "tcache.h"
#include "util.h"

void *my_malloc(size_t size) {
    if (size == 0) return NULL;

    ensure_global_init();

    arena_t *a = get_my_arena();

    if (!a || !a->base) return NULL;

    size_t payload = align_16(size);
    size_t need_total = align_16(sizeof(size_t) + payload);   // header + payload
    
    int bin = (int)(need_total / 16) - 2;   // 32->0, 48->1, 64->2 ... smallest normal is 32 (8 hdr + 16 payload -> 24 -> align -> 32)

    if (bin < 0 || bin >= TCACHE_MAX_BINS) bin = -1;

    // 1) Try per-thread tcache first
    void *hdr = NULL;

    if (bin >= 0) {
        tcache_bin_t *b = &g_tcache[bin];

        if (b->head != NULL) {
            free_chunk_t *fc = b->head;
            b->head = fc->fd;
            b->count--;
            hdr = (void*)fc;
        }
    }

    // 2) If tcache miss, fall back to arena freelist / bump
    if (!hdr) {
        pthread_mutex_lock(&a->lock);
        hdr = free_list_try(a, need_total);

        if (!hdr) {
            hdr = heap_carve_from_bump(a, need_total); // if free list miss, carve from top

            if (!hdr) {
                pthread_mutex_unlock(&a->lock);
                return NULL;  // out of arena
            }
        } 
        pthread_mutex_unlock(&a->lock);
    }

    void *ret = chunk_hdr_to_payload(hdr);
    
    return ret;
}

void my_free(void *ptr) {
    if (!ptr) return;

    ensure_global_init();

    arena_t *a = get_my_arena();
    if (!a || !a->base) return;

    uint8_t *hdr = (uint8_t*)chunk_payload_to_hdr(ptr);
    size_t csz = chunk_get_size(hdr);

    int bin = (int)(csz / 16) - 2;

    if (bin < 0 || bin >= TCACHE_MAX_BINS) bin = -1;

    // 1) Try to put small chunks into per-thread tcache
    if (bin >= 0) {
        tcache_bin_t *b = &g_tcache[bin];

        if (b->count < TCACHE_MAX_COUNT) {
            free_chunk_t *fc = (free_chunk_t*)hdr;

            // IMPORTANT: do NOT mark as free, do NOT set footer, do NOT coalesce.
            // Chunk stays "in-use" from the global allocator's point of view.
            
            fc->fd = b->head;
            b->head = fc;
            b->count++;
            
            return;
        }
    }

    // 2) Otherwise fall back to the old free path: mark free, coalesce, push into arena freelist.
    pthread_mutex_lock(&a->lock);

    chunk_write_size_to_hdr(hdr, csz);
    chunk_write_ftr(hdr, csz);

    void *merged = heap_coalesce_free_chunk(a, hdr);

    size_t msz = chunk_get_size(merged);
    uint8_t *merged_end = (uint8_t*)merged + msz;

    heap_set_next_chunk_P(a, merged, 0);

    // if the freed chunk touches the top, don't add to free list, shrink the unexplored region
    if (merged_end == a->bump) {
        a->bump = (uint8_t*)merged;
        pthread_mutex_unlock(&a->lock);
        return;
    }

    ((free_chunk_t*)merged)->fd = ((free_chunk_t*)merged)->bk = NULL;
    free_list_push_front(a, (free_chunk_t*)merged);
    
    pthread_mutex_unlock(&a->lock);
}