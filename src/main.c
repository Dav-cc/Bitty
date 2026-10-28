#include <stdint.h>
#include <stdlib.h>
#include "log.h"

#include "bencode.h"

int main(void){
    torrent_metadata* tm = calloc(1, sizeof(torrent_metadata));
    if(!tm){
        return -1;
    }

    error_t* err = calloc(1, sizeof(error_t));
    if(!err){
        return -1;
    }

    int res = bencode_parse_metadata("test/archlinux-2026.10.01-x86_64.iso.torrent", &tm, err);
    if(res !=0)
        return -1;

    return 0;
}
