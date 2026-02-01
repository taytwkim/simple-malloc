#include <sys/mman.h>   // for mmap
#include <stdio.h>
#include "arena.h"
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

heap_t* arena_add_new_heap(arena_t *a) {
    heap_t* h;
    return h;
}

void arena_init(arena_t *a, int id) {
    a->id = id;
    a->heaps = NULL;
    a->active_heap = NULL;
    a->free_list = NULL;
    pthread_mutex_init(&a->lock, NULL);

    heap_t *h = add_new_heap(a);
    
    if (!h) return;

    a->heaps = h;
    a->active_heap = h;
}

static void global_init(void) {
    #ifdef _OPENMP
        g_num_arenas = omp_get_max_threads();
    #else
        g_num_arenas = 1;
    #endif

    if (g_num_arenas < 1) g_num_arenas = 1;
    
    if (g_num_arenas > MAX_NUM_ARENAS) g_num_arenas = MAX_NUM_ARENAS;

    for (int i = 0; i < g_num_arenas; ++i) {
        arena_init(&g_arenas[i], i);
    }
}

arena_t *arena_from_thread(void) {
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

arena_t *arena_from_chunk_header(void *hdr) {
    arena_t *arena;
    return arena;
}