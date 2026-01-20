#include "arena.h"
#include "freelist.h"
#include "tcache.h"
#include "util.h"

void *my_malloc(size_t size) {
    if (size == 0) return NULL;

    arena_t *a = get_my_arena();
    if (!a || !a->base) return NULL;

    // if (DEBUG) printf("[malloc] entered: req=%zu [tid=%d]\n", size, omp_get_thread_num());

    size_t payload = align_16(size);
    size_t need = align_16(sizeof(size_t) + payload);  // header + payload
    size_t usable = need - sizeof(size_t);            // payload + padding for alignment

    int bin = size_to_tcache_bin(usable);

    // if (DEBUG && VERBOSE) printf("[malloc] aligned: payload=%zu (from %zu), need=%zu usable=%zu bin=%d\n", payload, size, need, usable, bin);

    // 1) Try per-thread tcache first
    void *hdr = NULL;

    if (bin >= 0) {
        tcache_bin_t *b = &g_tcache[bin];
        if (b->head != NULL) {
            free_chunk_t *fc = b->head;
            b->head = fc->links.fd;
            b->count--;

            hdr = (void*)fc;

            // if (DEBUG && VERBOSE) {
            //     uint8_t *payload_ptr = get_payload_from_hdr(hdr);
            //     uint8_t *chunk_end   = (uint8_t*)hdr + get_chunk_size(hdr);           
            //     printf("[malloc] from-tcache: hdr=%d payload=%d end=%d size=%zu\n", OFF(a, hdr), OFF(a, payload_ptr), OFF(a, chunk_end), get_chunk_size(hdr));
            // }
        }
    }

    // 2) If tcache miss, fall back to arena freelist / bump
    if (!hdr) {
        pthread_mutex_lock(&a->lock);
        hdr = try_free_list(a, need);

        if (!hdr) {
            // if (DEBUG && VERBOSE) printf("[malloc] freelist miss; carve from top; bump=%d\n", OFF(a, a->bump));

            hdr = carve_from_top(a, need); // carve from top

            if (!hdr) {
                pthread_mutex_unlock(&a->lock);
                return NULL;  // out of arena
            }

            // if (DEBUG && VERBOSE) { 
            //     uint8_t *payload_ptr = get_payload_from_hdr(hdr);
            //     uint8_t *chunk_end   = (uint8_t*)hdr + get_chunk_size(hdr);
            //     printf("[malloc] from-top: hdr=%d  payload=%d  end=%d  size=%zu  aligned=%d\n", OFF(a, hdr), OFF(a, payload_ptr), OFF(a, chunk_end), get_chunk_size(hdr), ((uintptr_t)payload_ptr & 15u) == 0);
            // }
        } 
        // else {
        //     if (DEBUG && VERBOSE) {
        //         uint8_t *payload_ptr = get_payload_from_hdr(hdr);
        //         uint8_t *chunk_end   = (uint8_t*)hdr + get_chunk_size(hdr);
        //         printf("[malloc] from-free-list: hdr=%d  payload=%d  end=%d  size=%zu  aligned=%d\n", OFF(a, hdr), OFF(a, payload_ptr), OFF(a, chunk_end), get_chunk_size(hdr), ((uintptr_t)payload_ptr & 15u) == 0);
        //     }
        // }
        pthread_mutex_unlock(&a->lock);
    }

    void *ret = get_payload_from_hdr(hdr);

    // if (DEBUG) printf("[malloc] exit: [tid=%d]\n", omp_get_thread_num());
    
    return ret;
}

void my_free(void *ptr) {
    if (!ptr) return;

    arena_t *a = get_my_arena();
    if (!a || !a->base) return;

    // if (DEBUG) printf("[free] entered: ptr=%d [tid=%d]\n", OFF(a, ptr), omp_get_thread_num());

    uint8_t *hdr = (uint8_t*)get_hdr_from_payload(ptr);
    size_t csz = get_chunk_size(hdr);
    size_t usable = csz - sizeof(size_t);
    int bin = size_to_tcache_bin(usable);

    // if (DEBUG && VERBOSE) printf("[free] header=%d, size=%zu usable=%zu bin=%d\n", OFF(a, hdr), csz, usable, bin);

    // 1) Try to put small chunks into per-thread tcache
    if (bin >= 0) {
        tcache_bin_t *b = &g_tcache[bin];

        if (b->count < TCACHE_MAX_COUNT) {
            free_chunk_t *fc = (free_chunk_t*)hdr;

            // IMPORTANT: do NOT mark as free, do NOT set footer, do NOT coalesce.
            // Chunk stays "in-use" from the global allocator's point of view.
            
            fc->links.fd = b->head;
            b->head = fc;
            b->count++;

            // if (DEBUG && VERBOSE) printf("[free] put into tcache bin=%d (count=%d)\n", bin, b->count);
            // if (DEBUG) printf("[free] exit (tcache): [tid=%d]\n", omp_get_thread_num());
            
            return;
        }
    }

    // 2) Otherwise fall back to the old free path: mark free, coalesce, push into arena freelist.
    pthread_mutex_lock(&a->lock);

    set_hdr_keep_prev(hdr, csz, 1);
    set_ftr(hdr, csz);

    void *merged = coalesce(a, hdr);

    size_t msz = get_chunk_size(merged);
    uint8_t *merged_end = (uint8_t*)merged + msz;

    set_next_chunk_hdr_prev(a, merged, 0);

    // if the freed chunk touches the top, don't add to free list, shrink the unexplored region
    if (merged_end == a->bump) {
        a->bump = (uint8_t*)merged;

        // if (DEBUG && VERBOSE) printf("[free] touches top; shrink: new bump=%d\n", OFF(a, a->bump));
        // if (DEBUG) printf("[free] exit (shrink): [tid=%d]\n", omp_get_thread_num());

        pthread_mutex_unlock(&a->lock);
        return;
    }

    ((free_chunk_t*)merged)->links.fd = ((free_chunk_t*)merged)->links.bk = NULL;
    push_front_to_free_list(a, (free_chunk_t*)merged);

    // if (DEBUG && VERBOSE) printf("[free] pushed to freelist: %d size=%zu\n", OFF(a, merged), msz);
    // if (DEBUG) printf("[free] exit: [tid=%d]\n", omp_get_thread_num());

    pthread_mutex_unlock(&a->lock);
}