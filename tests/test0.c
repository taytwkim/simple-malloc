#include <stdio.h>
#include <stdlib.h>
#include "../src/malloc.h"

int main(void) {
    // Calling malloc (intercepted by libsmalloc.so)
    void *p = malloc(17);
    
    // Explicitly print the result to ensure the call wasn't optimized away
    if (p) {
        printf("[test0] Allocated 17 bytes at address: %p\n", p);
    } else {
        printf("[test0] Malloc failed\n");
    }

    // Calling free
    free(p);
    
    printf("[test0] Finished successfully\n");
    return 0;
}