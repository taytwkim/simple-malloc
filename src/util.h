#ifndef MYALLOC_UTIL_H
#define MYALLOC_UTIL_H

#include <stddef.h>   // size_t
#include <stdint.h>   // uintptr_t
#include <unistd.h>   // getpagesize()

#define MYALLOC_ALIGNMENT 16

/* returns the pointer offset compared to arena base */
static inline int OFF(arena_t *a, void *p) {
    return (int)((uintptr_t)p - (uintptr_t)a->base);
}

/* requested size is rounded up to a multiple of 16 */
static inline size_t align_16(size_t n) {
    size_t rem = n % MYALLOC_ALIGNMENT;
    return rem == 0 ? n : (n + (MYALLOC_ALIGNMENT - rem));
}

/* returns whether the pointer's address is aligned to 16 */
static inline int is_aligned_16(const void *p) {
    return ((uintptr_t) p % MYALLOC_ALIGNMENT) == 0;
}

/* returns the OS page size casted as size_t instead of int */
static inline size_t pagesize(void) {
    return (size_t) getpagesize();
}

/* round n up to a multiple of the OS page size */
static inline size_t align_pagesize(size_t n) {
    size_t ps = pagesize();
    size_t rem = n % ps;
    return rem ? (n + (ps - rem)) : n;
}

/* returns min size needed to store a free chunk (need enough space for payload + footer) */
// static inline size_t get_free_chunk_min_size(void) {
//     size_t need = align_16(sizeof(free_chunk_t)) + sizeof(size_t);
//     return align_16(need);
// }

#endif