#include "tracker.h"
#include "bencode.h"
#include "log.h"
#include "byte.h"
#include "buffer.h"
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
    ti->announce = malloc(sizeof(char) * (tm->announce.length + 1)); // +1 for null terminator
    if(!ti->announce){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for malloc in tcti");
        free(ti);
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

    memcpy(ti->announce, tm->announce.data, tm->announce.length);
    ti->announce[tm->announce.length] = '\0'; // null terminate the string

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

int tracker_create_http_url(tracker_info* info, byte_buffer** out, error_t *err){
    char* url_path = malloc(URL_LEN_MAX);
    if(!url_path){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for malloc in tracker_create_http_url");
        return -1;
    }

    byte_buffer* buf = byte_buffer_init(URL_LEN_MAX, URL_LEN_MAX);
    if(!buf){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for buffer_create in tracker_create_http_url");
        free(url_path);
        return -1;
    }
    char port_str[6];
    char bdownload[20];
    char bleft[20];
    snprintf(port_str, sizeof(port_str), "%d", info->port);
    snprintf(bdownload, sizeof(bdownload), "%zu", info->downloaded);
    snprintf(bleft, sizeof(bleft), "%zu", info->left);


    byte_buffer_append(buf, info->announce, strlen(info->announce));
    byte_buffer_append(buf, "?info_hash=", 11);
    byte_buffer_append(buf, info->encoded_info_hash, strlen(info->encoded_info_hash));
    byte_buffer_append(buf, "&peer_id=", 9);
    byte_buffer_append(buf, info->encoded_peer_id, strlen(info->encoded_peer_id));
    byte_buffer_append(buf, "&port=", 6);
    byte_buffer_append(buf, port_str, strlen(port_str));
    byte_buffer_append(buf, "&download=", 10);
    byte_buffer_append(buf, bdownload, strlen(bdownload));
    byte_buffer_append(buf, "&left=", 6);
    byte_buffer_append(buf, bleft, strlen(bleft));

    log_msg(ll_info, "url created");

    *out = buf;
    return 0;
}
