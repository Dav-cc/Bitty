#include "tracker.h"
#include "bencode.h"
#include "log.h"
#include "byte.h"
#include "buffer.h"
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * TODO: Tracker new format for tracking host path and port seprate
 */

tracker_path* parse_tracker_url(const uint8_t* url, size_t url_len, error_t *err){
    size_t offset = 0;

    tracker_path* tp = malloc(sizeof(tracker_path));
    if(!tp){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for malloc in parse_tracker_url");
        return NULL;
    }
    const uint8_t* protocol = url;
    if(memmem(protocol, url_len, "http://", 7) == protocol){
        tp->prtcl = TRACKER_HTTP;
        offset += 7;
    }else if(memmem(protocol, url_len, "udp://", 6) == protocol){
        tp->prtcl = TRACKER_UDP;
        offset += 6;
    }else{
        err->any_err = 1;
        err->ecode = ERR_INVALID_URL;
        log_msg(ll_error, "invalid tracker url");
        free(tp);
        return NULL;
    }
    tp->host.data = url + offset;

    uint8_t* col = memmem(url + offset, url_len - offset, ":", 1);
    uint8_t* slash = memmem(url + offset, url_len - offset, "/", 1);
    if(!col || !slash || col > slash){
        err->any_err = 1;
        err->ecode = ERR_INVALID_URL;
        log_msg(ll_error, "invalid tracker url");
        free(tp);
        return NULL;
    }
    tp->host.length = col - (url + offset);
    offset += tp->host.length + 1;

    tp->port = strtol((const char*)(url + offset), NULL, 10);
    offset += strlen((const char*)(url + offset)) + 1;
    return tp;
}

int tracker_create_tracker_info(torrent_metadata* tm, uint8_t* peer_id, tracker_info** out, error_t *err){
    tracker_info* ti = malloc(sizeof(tracker_info));
    if(!ti){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for malloc in tcti");
        return -1;
    }

    ti->track_path = parse_tracker_url(tm->announce.data, tm->announce.length, err);
    if(!ti->track_path){
        goto cleanup;
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

int tracker_create_http_url(tracker_info* info, byte_buffer** out, error_t *err){
    byte_buffer* buf = byte_buffer_init(URL_LEN_MAX, URL_LEN_MAX);
    if(!buf){
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        log_msg(ll_error, "memory allocation failed for buffer_create in tracker_create_http_url");
        return -1;
    }
    char port_str[6];
    char bdownload[20];
    char bleft[20];
    char upload[20];
    snprintf(port_str, sizeof(port_str), "%d", info->port);
    snprintf(bdownload, sizeof(bdownload), "%zu", info->downloaded);
    snprintf(bleft, sizeof(bleft), "%zu", info->left);
    snprintf(upload, sizeof(upload), "%zu", info->uploaded);

    byte_buffer_append(buf, "/announce", 9);
    byte_buffer_append(buf, "?info_hash=", 10);
    byte_buffer_append(buf, info->encoded_info_hash, strlen(info->encoded_info_hash));
    byte_buffer_append(buf, "&peer_id=", 9);
    byte_buffer_append(buf, info->encoded_peer_id, strlen(info->encoded_peer_id));
    byte_buffer_append(buf, "&port=", 6);
    byte_buffer_append(buf, port_str, strlen(port_str));
    byte_buffer_append(buf, "&downloaded=", 12);
    byte_buffer_append(buf, bdownload, strlen(bdownload));
    byte_buffer_append(buf, "&uploaded=", 10);
    byte_buffer_append(buf, upload, strlen(upload));
    byte_buffer_append(buf, "&left=", 6);
    byte_buffer_append(buf, bleft, strlen(bleft));

    log_msg(ll_info, "url created");
    log_msg(ll_info, "%.*s", buf->offset, buf->data);

    *out = buf;
    return 0;
}

// byte_buffer* tracker_get_request(byte_buffer* buf){
//     int sfd = socket(AF_INET, SOCK_STREAM, 0);
//     if(sfd == -1){
//         return NULL;
//     }
//
//
// }
