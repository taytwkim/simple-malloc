#include <stdio.h>
#include <stdlib.h>
#include "../src/malloc.h"

int main(void) {
    void *p = malloc(16);
    
    // If we don't have this, compiler optimization might remove malloc
    if (p) {
        printf("[test0] Allocated 16 bytes at address: %p\n", p);
    } 
    else {
        printf("[test0] Malloc failed\n");
    }

    free(p);
    
    printf("[test0] Finished successfully\n");
    
    return 0;
}