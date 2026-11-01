#include <stdint.h>
#include <stdlib.h>
#include "log.h"
#include "byte.h"
#include "tracker.h"

#include "bencode.h"

int main(void){
    tracker_info *ti;
    uint8_t peer_id[20];
    byte_buffer* bb =NULL;
    torrent_metadata* tm = calloc(1, sizeof(torrent_metadata));
    if(!tm){
        return -1;
    }

    error_t* err = calloc(1, sizeof(error_t));
    if(!err){
        return -1;
    }

    int res = bencode_parse_metadata("test/debian-13.7.0-amd64-DVD-1.iso.torrent", &tm, err);
    if(res !=0)
        return -1;

    int pid_res = generate_peer_id(peer_id);
    if(pid_res < 0)
        return -1;

    int tracker_res = tracker_create_tracker_info(tm, peer_id, &ti, err);
    if(tracker_res != 0)
        return -1;

    int hres = tracker_create_http_url(ti, &bb, err);
    if(hres != 0)
        return -1;



    return 0;
}
