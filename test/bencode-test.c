#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../src/bencode.h"

int main(void)
{
    
    torrent_metadata* tm = bencode_parse_metadata("test/test.torrent");

    return 0;
}
