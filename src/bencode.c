#include "bencode.h"
#include "buffer.h"
#include "log.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>


static int8_t is_digit(byte_buffer *bb){
    if(bb->data[bb->offset] >= '0' && bb->data[bb->offset] <= '9')
        return 1;
    return 0;
}

static int8_t is_numeric_start(const uint8_t *data, uint8_t* is_neg){
    if(data[0] == '-')
        *is_neg = 1;
    if(data[0] == '-' || data[0] >= '0' && data[0] <= '9')
        return 1;
    return 0;
}
 
static byte_dict *dict_next(byte_dict *current)
{
    current->next_kv = calloc(1, sizeof(byte_dict));
    if (current->next_kv == NULL)
        return NULL;

    return current->next_kv;
}


static byte_list *list_next(byte_list *current)
{
    current->next_element = calloc(1, sizeof(byte_list));
    if (current->next_element == NULL)
        return NULL;

    return current->next_element;
}

// this function extracts integer values in a stream of bytes untill dlimeter
int extract_int_val(byte_buffer* bb, uint8_t dlim, int64_t* out, error_t* err){
    uint8_t is_negative = 0;
    if(out == NULL || bb == NULL || bb->data ==NULL || !is_numeric_start(bb->data + bb->offset, &is_negative)){
        log_msg(ll_error,"Offset error or buffer is NULL"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }


    int64_t value = 0;

    if (is_negative) {
      bb->offset++;
    }

    while(bb->offset < bb->len && bb->data[bb->offset] != dlim){
        uint8_t digit = bb->data[bb->offset] - '0';

        // value <= INT64_MAX => value * 10 + digit <= INT64_MAX => value <= (INT64_MAX - digit)/ 10
        if(value > (INT64_MAX - digit) / 10){
            log_msg(ll_error,"Overflow: value is bigger than INT64_MAX"); 
            err->any_err = 1;
            err->ecode = ERR_IO;
            return -1;
        }
        value = value * 10 + digit;
        bb->offset++;
    }
    if(is_negative){
        value = (-(value));
    }
    *out = value;
    return 0;
}

int bencode_parse_int(byte_buffer* bb, int64_t* out, error_t* err){
    int64_t val;
    if(bb == NULL ||bb->data == NULL || bb->data[bb->offset] != 'i'){
        log_msg(ll_error,"Offset error or buffer is NULL"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }

    bb->offset++;

    int res = extract_int_val(bb, 'e', &val, err);
    if(res == -1){
        // in here we got our error struct and not chnging it 
        // just simply return with -1
        return -1;
    }
    if (bb->offset >= bb->len || bb->data[bb->offset] != 'e') {
        log_msg(ll_error,"Offset error or buffer is NULL"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
      return -1;
    }

    *out = val;

    bb->offset++; // consume integer's 'e'

    return 0;
}

int bencode_parse_string(byte_buffer* bb, byte_slice** out, error_t* err){
    uint8_t* dc;
    int64_t slen = 0;
    if(bb == NULL || bb->len == 0 || bb->data == NULL || is_numeric_start(bb->data, dc)){
        log_msg(ll_error,"String Length Negative or not valid"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }
    int res = extract_int_val(bb, ':', &slen, err);
    if(res == -1){
        return -1;
    }
    if(bb->data[bb->offset] != ':'){
        log_msg(ll_error,"Invalid String Format"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }

    bb->offset++;

    byte_slice* bs = calloc(1, sizeof(byte_slice));
    if(!bs){
        log_msg(ll_error,"No Memory"); 
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        return -1;
    }
    bs->length = slen;
    bs->data = &bb->data[bb->offset];
    bb->offset += bs->length;

    *out = bs;
    return 0;
}

int bencode_parse_list(byte_buffer* bb,byte_list** out, error_t* err){
    if(bb == NULL || bb->data == NULL || bb->len == 0 ||bb->data[bb->offset] != 'l' ){
        log_msg(ll_error,"String Length Negative or not valid"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }

    bb->offset++;

    byte_list* bl = calloc(1, sizeof(byte_list));
    if(!bl){
        log_msg(ll_error,"No memory"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }
    byte_list* bl_head = bl;

    // whats the element type ?!

    while(bb->offset < bb->len && bb->data[bb->offset] != 'e'){
        if(bb->data[bb->offset] == 'i'){
            int64_t val;
            int res = bencode_parse_int(bb, &val, err);

            if(res == -1){
                return -1;
            }
            bl->type = BENCODE_INT;
            bl->i = val;

            bl = list_next(bl);
            if(!bl){
                log_msg(ll_error, "No memory");
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }
            continue;
        }
        else if(bb->data[bb->offset] == 'l'){
            byte_list* bll;
            int res = bencode_parse_list(bb, &bll, err); 
            if(res == -1){
                return -1;
            }
            bl->type = BENCODE_LIST;
            bl->l = bll;

            bl = list_next(bl);
            if(!bl){

                log_msg(ll_error, "No memory");
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }

            continue;
        }
        else if(bb->data[bb->offset] == 'd'){
            byte_dict* bd;
            int res = bencode_parse_dict(bb, &bd, err);
            if(!res)
                return -1;

            bl->type = BENCODE_DICT;
            bl->d = bd;

            bl = list_next(bl);
            if(!bl){

                log_msg(ll_error, "No memory");
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }

            continue;
        }
        else if(is_digit(bb)){
            byte_slice* bs;
            int res = bencode_parse_string(bb, &bs, err);
            if(res == -1){
                return -1;
            }

            bl->type = BENCODE_STRING;
            bl->s = *bs;

            bl = list_next(bl);
            if(!bl){
                log_msg(ll_error, "No memory");
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }

            continue;
        }
        else{
            log_msg(ll_error,"Not Valid Bencode"); 
            err->any_err = 1;
            err->ecode = ERR_BENCODE_INVALID;
            return -1;
        }
    }

    if (bb->offset >= bb->len) {
            log_msg(ll_error,"offset bigger than length"); 
            err->any_err = 1;
            err->ecode = ERR_BENCODE_INVALID;
            return -1;
    }
    bb->offset++;
    *out = bl_head;
    return 0;
}

int bencode_parse_dict(byte_buffer* bb, byte_dict** out, error_t* err){
    if(bb == NULL || bb->data == NULL || bb->len == 0|| bb->offset > bb->len ||bb->data[bb->offset] != 'd'){
            log_msg(ll_error,"Offset error or buffer is NULL"); 
            err->any_err = 1;
            err->ecode = ERR_BENCODE_INVALID;
            return -1;

    }
    bb->offset++;

    // we are on the first key now and keys are string types
    byte_dict* bd_head = calloc(1, sizeof(byte_dict));
    if(bd_head == NULL){
        log_msg(ll_error,"No Memory for bd_head"); 
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        return -1;
    }
    byte_dict* bd = bd_head;

    while(bb->len > bb->offset && bb->data[bb->offset] != 'e'){

        int res = bencode_parse_string(bb, &bd->key, err);

        if (res == -1|| bb->offset >= bb->len){
            return -1;
        }

        if(bb->data[bb->offset] == 'i'){
            int64_t val;
            int res = bencode_parse_int(bb, &val, err);
            if(res == -1){
                return -1;
            }
            bd->type_val = BENCODE_INT;
            bd->val.i = val;

            if(bb->len > bb->offset && bb->data[bb->offset] != 'e'){
                bd = dict_next(bd);
                if(bd == NULL){
                    log_msg(ll_error,"out of memory"); 
                    err->any_err = 1;
                    err->ecode = ERR_OUT_OF_MEMORY;
                    return -1;
                }
            }
            continue;
        }
        else if(bb->data[bb->offset] == 'l'){
            byte_list* bll ;
            int res = bencode_parse_list(bb, &bll, err); 
            if(res == -1){
                return -1;
            }
            bd->type_val = BENCODE_LIST;
            bd->val.l = bll;

            if(bb->len > bb->offset && bb->data[bb->offset] != 'e'){
                bd = dict_next(bd);
                if(bd == NULL){
                    log_msg(ll_error,"out of memory"); 
                    err->any_err = 1;
                    err->ecode = ERR_OUT_OF_MEMORY;
                    return -1;
                }
            }
            continue;
        }
        else if(bb->data[bb->offset] == 'd'){
            bd->val_start = bb->offset;
            byte_dict* bdd;
            int res = bencode_parse_dict(bb, &bdd, err);
            bd->val_end = bb->offset - 1;
            if(bdd == NULL){
                log_msg(ll_error,"out of memory"); 
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }
            bd->type_val = BENCODE_DICT;
            bd->val.d = bdd;

            bd = dict_next(bd);
            if (bd == NULL){
                log_msg(ll_error,"out of memory"); 
                err->any_err = 1;
                err->ecode = ERR_OUT_OF_MEMORY;
                return -1;
            }
            continue;
        }
        else if (is_digit(bb)) {
            byte_slice *bs;
            int res = bencode_parse_string(bb, &bs, err);
            if (res == -1) {
                return -1;
            }
            bd->type_val = BENCODE_STRING;
            bd->val.s = *bs;

            if(bb->len > bb->offset && bb->data[bb->offset] != 'e'){
                bd = dict_next(bd);
                if(bd == NULL){
                    log_msg(ll_error,"out of memory"); 
                    err->any_err = 1;
                    err->ecode = ERR_OUT_OF_MEMORY;
                    return -1;

                }
            }
            continue;
        }
        else{
            log_msg(ll_error,"Not a valid format"); 
            err->any_err = 1;
            err->ecode = ERR_BENCODE_INVALID;
            return -1;
        }
    }
    if (bb->offset >= bb->len) {
        log_msg(ll_error,"Offset error"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }
    bb->offset++; // consume 'e'
    *out = bd_head;
    return 0;
}


int find_dict_val(byte_dict* bd, const char* key, byte_dict** out, error_t* err){
    if(bd == NULL){
        log_msg(ll_error,"dict is NULL"); 
        err->any_err = 1;
        err->ecode = ERR_BENCODE_INVALID;
        return -1;
    }

    size_t key_len = strlen(key);

    while(bd != NULL){
        if(((bd->key->length == key_len)) && (memcmp(bd->key->data, key, bd->key->length) == 0)){
            *out = bd;
            return 0;
        }
        bd = bd->next_kv;
        continue;
    }
    return -1;
}


int bencode_parse_metadata(const char* filepath, torrent_metadata* out, error_t* err){
    byte_dict *pdict;
    byte_dict *pinfo;

    byte_buffer* bb = byte_buffer_read_file(filepath);
    if(bb == NULL){
        log_msg(ll_error,"Error in read file %s", filepath); 
        err->any_err = 1;
        err->ecode = ERR_IO;
        return -1;

    }

    byte_dict* bd;
    int res = bencode_parse_dict(bb, &bd, err);
    if(res == -1){
        byte_buffer_destroy(bb);
        return -1;
    }

    torrent_metadata* tm= calloc(1, sizeof(torrent_metadata));
    if(!tm){
        byte_buffer_destroy(bb);
        free(bd);
        log_msg(ll_error, "error init memory");
        err->any_err = 1;
        err->ecode = ERR_OUT_OF_MEMORY;
        return -1;
    }

    // BUG: this output is NULL
    int result1  = find_dict_val(bd, "announce", &pdict, err);
    if(result1 == 0 && pdict->type_val == BENCODE_STRING){
            tm->announce.data = pdict->val.s.data;
            tm->announce.length = pdict->val.s.length;

    }

    int result2 = find_dict_val(bd, "info", &pdict, err);

    if (result2 == 0 && pdict->type_val == BENCODE_DICT) {
        byte_dict* pinfo = pdict->val.d;

        SHA1(&bb->data[pdict->val_start], pdict->val_end - pdict->val_start, tm->info_hash);

            int fdvn = find_dict_val(pinfo, "name", &pdict, err);
            if(fdvn == 0 && pdict->type_val == BENCODE_STRING){
                        tm->info.name.data = pdict->val.s.data;
                        tm->info.name.length = pdict->val.s.length;

            }
            int fdvnp= find_dict_val(pinfo, "pieces", &pdict, err);
                if (pdict == 0 && pdict->type_val == BENCODE_STRING) {
                    tm->info.peices.data = pdict->val.s.data;
                    tm->info.peices.length = pdict->val.s.length;
            }
            int fdvpl = find_dict_val(pinfo, "piece length", &pdict, err);
                if (fdvpl == 0  && pdict->type_val == BENCODE_INT) {
                    tm->info.piece_length = pdict->val.i;
            }
            int fdvl = find_dict_val(pinfo, "length", &pdict, err);
                if (fdvl == 0 && pdict->type_val == BENCODE_INT) {
                    tm->info.length = pdict->val.i;
                }
    }
    log_msg(ll_info, "metadata parsed succesfully");
    log_msg(ll_info, "announce:%.*s", tm->announce.length,tm->announce.data);
    log_msg(ll_info, "info.length:%zu", tm->info.length);
    log_msg(ll_info, "info.name:%.*s", tm->info.name.length, tm->info.name.data);
    return 0;
}
