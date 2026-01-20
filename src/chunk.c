#include "arena.h"
#include "chunk.h"
#include "util.h"

void set_prev_bit_in_hdr(void *hdr, int on) {
    size_t h = *(size_t*)hdr;
    if (on) h |=  CHUNK_PREV_IN_USE_BIT;
    else h &= ~CHUNK_PREV_IN_USE_BIT;
    *(size_t*)hdr = h;
}

/* set free bit and return header */
size_t build_hdr_with_free_bit(size_t size_aligned, int is_free) {
    size_t s = size_aligned & CHUNK_SIZE_MASK;   // keep high bits only
    return is_free ? (s | CHUNK_FREE_BIT) : (s & ~CHUNK_FREE_BIT);
}

void set_hdr_keep_prev(void *hdr, size_t size_aligned, int is_free) {
    // set header, but don't update the prev_in_use_bit
    size_t old   = *(size_t*)hdr;
    size_t prevb = old & CHUNK_PREV_IN_USE_BIT;
    *(size_t*)hdr = build_hdr_with_free_bit(size_aligned, is_free) | prevb;
}

void set_ftr(void *hdr, size_t size_aligned) {
    uint8_t *base = (uint8_t*)hdr;
    *(size_t*)(base + size_aligned - sizeof(size_t)) = build_hdr_with_free_bit(size_aligned, 1);
}

void* get_next_chunk_hdr(void *hdr) { 
    return (uint8_t*)hdr + get_chunk_size(hdr); 
}

/*  return min size of a free chunk (need enough space for free chunk + footer) */
size_t get_free_chunk_min_size(void) {
    size_t need = align_16(sizeof(free_chunk_t)) + sizeof(size_t);
    return align_16(need);
}