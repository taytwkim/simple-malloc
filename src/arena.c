#include <sys/mman.h>   // for mmap
#include <stdio.h>
#include "arena.h"
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

int arena_add_new_heap(arena_t *a) {
    size_t req = align_pagesize(g_arena_bytes);

    void *mem = mmap(NULL, req, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (mem == MAP_FAILED) return -1;

    heap_t *h = (heap_t *)mem;
    h->arena = a;
    h->next  = NULL;
    
    uint8_t *payload = (uint8_t *)mem + sizeof(*h);

    h->base = payload;
    h->bump = h->base;
    h->end = (uint8_t *)mem + req;

    // Append instead of overwrite.
    if (a->heaps == NULL) {
        a->heaps = h;
    } 
    else {
        heap_t *curr = a->heaps;
        while (curr->next != NULL) curr = curr->next;
        curr->next = h;
    }

    a->active_heap = h; // For now, let's say active heap is always the most recently added heap.

    return 0;
}

/*
 * TO DO: Update arena_init to return an int instead of void.
 * Prevent arena_init from failing silently.
 */
int arena_init(arena_t *a, int id) {
    a->id = id;
    a->heaps = NULL;
    a->active_heap = NULL;
    a->free_list = NULL;
    pthread_mutex_init(&a->lock, NULL);

    int add_heap_succeeded = arena_add_new_heap(a);
    if (add_heap_succeeded < 0) return -1;

    return 0;
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

void ensure_global_init(void) {
    pthread_once(&g_once, global_init);
}

/* 
 * TO DO: We have to fix arena_from_thread and arena_from_chunk_header
 */
arena_t *arena_from_thread(void) {
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
    arena_t* a = arena_from_thread();
    return a;
}