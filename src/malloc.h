#ifndef MY_ALLOC_H
#define MY_ALLOC_H

#include <stddef.h>

void *my_malloc(size_t size);
void my_free(void *ptr);

#endif