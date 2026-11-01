#ifndef BITTY_TRACKER_H
#define BITTY_TRACKER_H

// this struct owns pointe fields inside it
#include "bencode.h"

typedef struct{
    char* encoded_info_hash;
    char* encoded_peer_id;

    uint16_t port;

    uint64_t uploaded;
    uint64_t downloaded;
    uint64_t left;

} tracker_info;

// this function takes tm and peer id (raw bytes) and make Http Get request url to sent for nodes 
int tracker_create_tracker_info(torrent_metadata* tm,uint8_t* peer_id, tracker_info** out, error_t *err);


#endif
