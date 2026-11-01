#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*
 *  Byte Percent Encodeing:
 *      is type of encoding mostly url's in a way that encode 8-bit characters that have specific meanings (using '%' followed by two hexadecimal digit)
 *      in this case encoding binary values like info_hash or peer_id in a http tracker request 
 */

char* raw_byte_to_percent_encoding(uint8_t* data, size_t size){
    // each byte will be represented by 3 characters: '%' + 2 hex digits
    size_t encoded_size = size * 3 + 1; // +1 for null terminator

    // caller function is responsible for freeing memory
    char* encoded = (char*)malloc(encoded_size);
    if (!encoded) {
        log_msg(ll_error, "Memory allocation failed for percent encoding");
        return NULL; // Memory allocation failed
    }

    for (size_t i = 0; i < size; i++) {
        sprintf(encoded + i * 3, "%%%02X", data[i]);
    }
    encoded[encoded_size - 1] = '\0'; // Null-terminate the string

    return encoded;
}



