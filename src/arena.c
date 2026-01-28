#include <sys/mman.h>
#include <stdio.h>
#include "arena.h"
#include "debug.h"
#include "freelist.h"
#include "util.h"

// If OpenMP is enabled (-fopenmp), the compiler defines _OPENMP
#ifdef _OPENMP
    #include <omp.h>
#endif

static pthread_once_t g_once = PTHREAD_ONCE_INIT;
static arena_t g_arenas[MAX_NUM_ARENAS];
static int g_num_arenas = 0;
static size_t g_arena_bytes = (size_t)(16 * 1024 * 1024);

// If compiled with a specific C standard, the compiler defines __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
    static _Thread_local arena_t *t_arena = NULL;
#else
    static __thread arena_t *t_arena = NULL;
#endif

void arena_init(arena_t *a, int id) {
    size_t req = align_pagesize(g_arena_bytes);

    void *mem = mmap(NULL, req, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (mem == MAP_FAILED) {
        a->base = a->bump = a->end = NULL;
        a->free_list = NULL;
        return;
    }

    a->id = id;
    a->base = (uint8_t*)mem;
    a->bump = a->base;
    a->end = a->base + req;
    a->free_list = NULL;

    pthread_mutex_init(&a->lock, NULL);

    DBG("[arena_init] id=%d base=%p bump=%p end=%p bytes=%zu\n", id, (void*)a->base, (void*)a->bump, (void*)a->end, req);
}

static void global_init(void) {
#ifdef _OPENMP
    g_num_arenas = omp_get_max_threads();
#else
    g_num_arenas = 1;
#endif

    if (g_num_arenas < 1) g_num_arenas = 1;
    if (g_num_arenas > MAX_NUM_ARENAS) g_num_arenas = MAX_NUM_ARENAS;

    for (int i = 0; i < g_num_arenas; i++) {
        arena_init(&g_arenas[i], i);
    }
}

arena_t *get_my_arena(void) {
    pthread_once(&g_once, global_init);

    if (t_arena) return t_arena;

    int tid = 0;

#ifdef _OPENMP
    tid = omp_get_thread_num();
    if (tid < 0) tid = 0;
#endif

    int idx = tid % g_num_arenas;
    t_arena = &g_arenas[idx];
    return t_arena;
}

void arena_set_next_chunk_P(arena_t *a, void *hdr, int prev_in_use) {
    void *nxt = get_next_chunk_hdr(hdr);
    if ((uint8_t*)nxt < a->bump) chunk_set_P(nxt, prev_in_use);
}

void* arena_carve_from_top(arena_t *a, size_t need_total) {
    uintptr_t start = (uintptr_t)a->bump;

    uintptr_t payload = (start + sizeof(size_t) + 15u) & ~((uintptr_t)15u);
    uint8_t *hdr = (uint8_t*)(payload - sizeof(size_t));

    if ((size_t)(a->end - hdr) < need_total) return NULL;

    chunk_write_size_to_hdr(hdr, need_total);
    chunk_set_P(hdr, 1);

    a->bump = hdr + need_total;

    return hdr;
}

void* arena_coalesce_free_chunk(arena_t *a, void *hdr) {
    size_t csz = chunk_get_size(hdr);
    void *nxt = get_next_chunk_hdr(hdr);

    if ((uint8_t*)nxt < a->bump && chunk_is_free(nxt)) {
        DBG("[coalesce] right chunk is free, merge with right chunk\n");

        size_t nxt_sz = chunk_get_size(nxt);
        free_list_remove(a, (free_chunk_t*)nxt);
        csz += nxt_sz;
        chunk_write_size_to_hdr(hdr, csz);
        chunk_write_ftr(hdr, csz);
    }

    if (prev_chunk_is_free(hdr)) {
        DBG("[coalesce] left chunk is free, merge with left chunk\n");

        uint8_t *p = (uint8_t*)hdr;
        void* prev_footer = (p - sizeof(size_t));
        size_t prev_sz = chunk_get_size(prev_footer);
        void *prv = p - prev_sz;
        free_list_remove(a, (free_chunk_t*)prv);
        csz += prev_sz;
        chunk_write_size_to_hdr(prv, csz);
        chunk_write_ftr(prv, csz);
        hdr = prv;
    }
    return hdr;
}

void* arena_split_free_chunk(arena_t *a, free_chunk_t *fc, size_t need_total) {
    size_t csz = chunk_get_size(fc);
    const size_t MIN_FREE = get_free_chunk_min_size();

    if (csz >= need_total + MIN_FREE) {
        free_list_remove(a, fc);

        uint8_t *base = (uint8_t*)fc;
        chunk_write_size_to_hdr(base, need_total);
        arena_set_next_chunk_P(a, base, 1);

        uint8_t *rem = base + need_total;
        size_t rem_sz = csz - need_total;

        chunk_write_size_to_hdr(rem, rem_sz);
        chunk_write_ftr(rem, rem_sz);

        ((free_chunk_t*)rem)->fd = ((free_chunk_t*)rem)->bk = NULL;
        free_list_push_front(a, (free_chunk_t*)rem);

        return base;
    } 
    else {
        free_list_remove(a, fc);
        chunk_write_size_to_hdr(fc, csz);
        arena_set_next_chunk_P(a, fc, 1);
        return fc;
    }
}