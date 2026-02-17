#ifndef MY_ALLOC_H
#define MY_ALLOC_H

#include <stddef.h> // for size_t

void *malloc(size_t size);
void free(void *ptr);

#endif