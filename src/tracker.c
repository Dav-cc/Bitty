#include "tracker.h"
#include "bencode.h"
#include "log.h"
#include "byte.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


int tracker_create_tracker_info(torrent_metadata* tm, uint8_t* peer_id, tracker_info** out, error_t *err){
    tracker_info* ti = malloc(sizeof(tracker_info));
    if(!ti){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for malloc in tcti");
        return -1;
    }
    // default torrent port
    ti->port = 6881;

    ti->downloaded = 0;
    ti->uploaded = 0;

    ti->encoded_info_hash = raw_byte_to_percent_encoding(tm->info_hash, 20);
    if(ti->encoded_info_hash == NULL){
        goto cleanup;
    }

    ti->encoded_peer_id = raw_byte_to_percent_encoding(peer_id, 20);
    if(ti->encoded_peer_id == NULL){
        goto cleanup;
    }

    ti->left = tm->info.length;

    *out = ti;
    return 0;

cleanup:
    err->any_err = 1;
    err->ecode = ERR_OUT_OF_MEMORY;
    log_msg(ll_error, "memory allocation failed");
    free(ti->encoded_info_hash);
    free(ti->encoded_peer_id);
    free(ti);
    return -1;
}

