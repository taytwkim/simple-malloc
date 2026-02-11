#ifndef MYALLOC_CHUNK_H
#define MYALLOC_CHUNK_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t
#include "util.h"   // align_16

/* 
 * In-use:    [ header (size | flags) ]       8 bytes (in a 64 bit machine), the last four bits are flags
 *            [ owning heap ptr       ]       8 bytes
 *            [ payload ...           ]
 * 
 * Free:      [ header (size | flags) ]       8 bytes
 *            [ owning heap ptr       ]       8 bytes
 *            [ fd                    ]       8 bytes, forward pointer to the next free chunk
 *            [ bk                    ]       8 bytes, backward pointer to the prev free chunk
 *            ... 
 *            [ footer (size )        ]       8 bytes, same as the header, but flag bits are zeros
 * 
 * flags: 
 *    - bit 0: PREV_IN_USE_BIT (P)
 * 
 * Note: the reason why we can store the chunk size and the flags in a single header is because the chunk size is 16 aligned in a 64-bit machine.
 * This means that the low four bits of the chunk size will always be zero - so we can use these bits to store metadata.
 */

typedef struct heap heap_t;

typedef struct chunk_prefix {
    size_t hdr;
    heap_t *heap;
} chunk_prefix_t;

typedef struct free_chunk {
    size_t hdr;
    struct free_chunk *prev, *next;
} free_chunk_t;

// CHUNK_HDR_SIZE_MASK clears the flag bits (…FFF0)
//    - header & CHUNK_HDR_SIZE_MASK → size
// 0xFUL is hex 15 (F) with the UL suffix (unsigned long), and notice that we flip the bits (~).
#define CHUNK_HDR_SIZE_MASK (~(size_t)0xF)

/* 
 * PREV_IN_USE_BIT = mask for bit 0 (…0001)
 *   - Set  : header |=  CHUNK_PREV_IN_USE_BIT → previous chunk is IN-USE
 *   - Clear: header &= ~CHUNK_PREV_IN_USE_BIT → previous chunk is FREE
 * 
 * Why do we need this flag?
 * 
 * We need this flag when merging two chunks. When a chunk is freed, we look at its left neighbor and try to merge.
 * But we want to first make sure that the left chunk is actually free. 
 * If we naively read from the left chunk's footer without checking, we might be reading from the payload of an in-use chunk.
 * This is not really a problem when merging with the right chunk, because both free and in-use chunks have the header.
 */
#define CHUNK_HDR_P_MASK ((size_t) 1)

static inline int chunk_get_P(size_t hdr_word) { return (hdr_word & CHUNK_HDR_P_MASK) != 0; }   // hdr_word differentiated from size_t* hdr

static inline void chunk_set_P(void *hdr, int on) {
    size_t h = *(size_t*)hdr;
    if (on) h |=  CHUNK_HDR_P_MASK;
    else h &= ~CHUNK_HDR_P_MASK;
    *(size_t*)hdr = h;
}

static inline void chunk_write_size_to_hdr(void *hdr, size_t size_aligned) {
    size_t flag = *(size_t*)hdr & CHUNK_HDR_P_MASK;
    *(size_t*)hdr = (size_aligned & CHUNK_HDR_SIZE_MASK) | flag;
}

static inline void chunk_write_ftr(void *hdr, size_t size_aligned) {
    uint8_t *base = (uint8_t*)hdr;
    *(size_t*)(base + size_aligned - sizeof(size_t)) = (size_aligned & CHUNK_HDR_SIZE_MASK);
}

static inline uint8_t* chunk_hdr_to_payload(void *hdr) { return (uint8_t*)hdr + sizeof(size_t); }

static inline void* chunk_payload_to_hdr(void *ptr) { return (uint8_t*)ptr - sizeof(size_t); }

static inline size_t chunk_get_size(void *hdr) { return (*(size_t*)hdr) & CHUNK_HDR_SIZE_MASK; }

static inline int prev_chunk_is_free(void *hdr) { return !chunk_get_P(*(size_t*)hdr); }

static inline void* get_next_chunk_hdr(void *hdr) { return (uint8_t*)hdr + chunk_get_size(hdr); }

static inline int chunk_is_free(void *hdr) {
    void *nxt = get_next_chunk_hdr(hdr);
    return prev_chunk_is_free(nxt);
}

static inline size_t get_free_chunk_min_size(void) { return align_16(sizeof(free_chunk_t) + sizeof(size_t)); }

#endif