#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../src/bencode.h"

int main(void)
{
    const uint8_t input[] = "d4:name5:David4:agei20e4:listli1ei2eee";

    byte_buffer bb = {
        .data = (uint8_t *)input,
        .len = sizeof(input) - 1,
        .offset = 0
    };

    printf("Input: %.*s\n", (int)bb.len, bb.data);
    printf("Length: %zu\n", bb.len);

    bencode_parse_dict(&bb);

    return 0;
}
