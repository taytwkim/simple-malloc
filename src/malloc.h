#ifndef MY_ALLOC_H
#define MY_ALLOC_H

#include <stddef.h> // for size_t

void *my_malloc(size_t size);
void my_free(void *ptr);

#endif