#ifndef MYALLOC_CHUNK_H
#define MYALLOC_CHUNK_H

#include "arena.h"
#include <stddef.h>
#include <stdint.h>

/* 
 * In-use:    [ header (size | flags) ]       8 bytes (in a 64 bit machine), the last four bits are flags
 *            [ payload ...           ]
 * 
 * Free:      [ header (size | flags) ]       8 bytes
 *            [ fd                    ]       8 bytes, forward pointer to the next free chunk
 *            [ bk                    ]       8 bytes, backward pointer to the prev free chunk
 *            ... 
 *            [ footer (size | flags) ]       8 bytes, same as the header, but CHUNK_PREV_IN_USE_BIT is not actively updated
 * 
 * flags: 
 *    - bit 0: CHUNK_FREE_BIT
 *    - bit 1: CHUNK_PREV_IN_USE_BIT
 * 
 * Note: the reason why we can store the chunk size and the flags in a single header is because the chunk size is 16 aligned.
 * This means that the low four bits of the chunk size will always be zero - so we can use these bits to store metadata.
 */

typedef struct free_links { 
    struct free_chunk *fd, *bk; 
} free_links_t;

typedef struct free_chunk {
    size_t size_and_flags;        // total size (incl header; footer exists only when free)
    free_links_t links;           // valid only when free (lives at start of payload)
} free_chunk_t;

// CHUNK_SIZE_MASK clears the flag bits (…FFF0)
//    - header & CHUNK_SIZE_MASK → size
#define CHUNK_SIZE_MASK (~(size_t) 0xFUL)

// CHUNK_FREE_BIT = mask for bit 0 (…0001)
//   - Set  : header |=  CHUNK_FREE_BIT → mark chunk FREE
//   - Clear: header &= ~CHUNK_FREE_BIT → mark chunk IN-USE
#define CHUNK_FREE_BIT ((size_t) 1u)

/* 
 * CHUNK_PREV_IN_USE_BIT = mask for bit 1 (…0010)
 *   - Set  : header |=  CHUNK_PREV_IN_USE_BIT → previous chunk is IN-USE
 *   - Clear: header &= ~CHUNK_PREV_IN_USE_BIT → previous chunk is FREE
 * 
 * Why do we need this flag?
 * 
 * We need this flag when merging two chunks. When a chunk is freed, we look at its left neighbor and try to merge.
 * But we want to first make sure that the left chunk is actually free. If we naively read from the left chunk's footer without checking, 
 * we might be reading from the payload of an in-use chunk.
 * This is not really a problem when merging with the right chunk, because both free and in-use chunks have the header
 */
#define CHUNK_PREV_IN_USE_BIT ((size_t) 2u)

/* Header & Footer Operations */
static inline size_t get_size_from_hdr(size_t hdr) { return hdr & CHUNK_SIZE_MASK; }

static inline int get_free_bit_from_hdr(size_t hdr) { return (int)(hdr & CHUNK_FREE_BIT); }

static inline int get_prev_from_hdr(size_t hdr_word) { return (int)(hdr_word & CHUNK_PREV_IN_USE_BIT); }

void set_prev_bit_in_hdr(void *hdr, int on);

size_t build_hdr_with_free_bit(size_t size_aligned, int is_free);

void set_hdr_keep_prev(void *hdr, size_t size_aligned, int is_free);

void set_ftr(void *hdr, size_t size_aligned);

/* Pointer Conversion Between Header and Payload */

static inline uint8_t* get_payload_from_hdr(void *hdr) { return (uint8_t*)hdr + sizeof(size_t); }

static inline void* get_hdr_from_payload(void *ptr) { return (uint8_t*)ptr - sizeof(size_t); }

/* Chunk Operations */

static inline size_t get_chunk_size(void *hdr) { return get_size_from_hdr(*(size_t*)hdr); }

static inline int chunk_is_free(void *hdr) { return get_free_bit_from_hdr(*(size_t*)hdr); }

static inline int prev_chunk_is_free(void *hdr) { return !get_prev_from_hdr(*(size_t*)hdr); }

void* get_next_chunk_hdr(void *hdr);

size_t get_free_chunk_min_size(void);

#endif