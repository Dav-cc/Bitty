#ifndef BITTY_TRACKER_H
#define BITTY_TRACKER_H

// this struct owns pointe fields inside it
#include "bencode.h"
#include "buffer.h"

#define URL_LEN_MAX 4096

typedef enum{
    TRACKER_HTTP,
    TRACKER_UDP
} tracker_protocol;

typedef struct {
    byte_slice announce; // whole annouce url like (http://bttracker.debian.org:6969/announce)
    byte_slice path;     // path of tracker in url (the /announce part)
    uint16_t port;  // port that tracker listening at
    byte_slice host;     // url host(bttracker.debian.org)
    tracker_protocol prtcl; // http or udp types of trakcer (utp is not supported yet)
} tracker_path;


typedef struct{
    tracker_path* track_path;
    char* encoded_info_hash;
    char* encoded_peer_id;

    uint16_t port;

    uint64_t uploaded;
    uint64_t downloaded;
    uint64_t left;

} tracker_info;


// this function takes tm and peer id (raw bytes) and make Http Get request url to sent for nodes 
int tracker_create_tracker_info(torrent_metadata* tm,uint8_t* peer_id, tracker_info** out, error_t *err);
int tracker_create_http_url(tracker_info* info, byte_buffer** out, error_t *err);


#endif
