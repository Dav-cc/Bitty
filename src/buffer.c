#include "buffer.h"
#include <stdlib.h>

byte_buffer* byte_buffer_init(size_t cap){
    if(cap == 0)
        return NULL;

    byte_buffer* bb = calloc(1, sizeof(byte_buffer));
    if(!bb)
        return NULL;

    // it should be stat.st_size length but for now 2048 is enough
    bb->data = calloc(1, 2048);
    if(!bb)
        return NULL;

    bb->cap = cap;
    return bb;
}

void byte_buffer_destroy(byte_buffer* bb){
    free((void*)bb->data);
    bb->data = NULL;
    bb->len = 0;
    bb->offset = 0;
    bb->cap = 0;
    free((void*)bb);
}
