#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../src/malloc.h"

int main(void){
    void *p = my_malloc(17);
    my_free(p);
    
    return 0;
}