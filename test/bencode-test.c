#include <stdint.h>
#include <stdlib.h>
#include "../src/log.h"

#include "../src/bencode.h"

int main(void){
    torrent_metadata* tm = calloc(1, sizeof(torrent_metadata));
    if(!tm){
        return -1;
    }

    error_t* err = calloc(1, sizeof(error_t));
    if(!err){
        return -1;
    }

    int res = bencode_parse_metadata("test/test.torrent", &tm, err);

    return 0;
}
