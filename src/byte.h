#ifndef BITTY_BYTE_H
#define BITTY_BYTE_H
#include <stdint.h>
#include <stdlib.h>

#define PEERID_LEN  20

char* raw_byte_to_percent_encoding(uint8_t* data, size_t size);
int generate_peer_id(uint8_t* pid);

#endif
