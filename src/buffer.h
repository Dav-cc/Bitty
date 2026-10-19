#ifndef BITTY_BUFFER_H
#define BITTY_BUFFER_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    const uint8_t* data;
    size_t cap;
    size_t len;
    size_t offset;
} byte_buffer;

byte_buffer* byte_buffer_init(size_t cap);
void byte_buffer_destroy(byte_buffer* bb);

#endif //BITTY_BUFFER_H
