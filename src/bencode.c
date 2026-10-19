#include "bencode.h"
#include "buffer.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

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
    current->next_element = calloc(1, sizeof(byte_dict));
    if (current->next_element == NULL)
        return NULL;

    return current->next_element;
}

// this function extracts integer values in a stream of bytes untill dlimeter
int8_t extract_int_val(byte_buffer *bb, uint8_t dlim, int64_t* out){
    uint8_t is_negative = 0;
    if(out == NULL || bb == NULL || bb->data ==NULL || !is_numeric_start(bb->data + bb->offset, &is_negative))
        return -1;

    int64_t value = 0;

    if (is_negative) {
      bb->offset++;
    }

    while(bb->offset < bb->len && bb->data[bb->offset] != dlim){
        uint8_t digit = bb->data[bb->offset] - '0';

        // value <= INT64_MAX => value * 10 + digit <= INT64_MAX => value <= (INT64_MAX - digit)/ 10
        if(value > (INT64_MAX - digit) / 10){
            return -1;
        }
        value = value * 10 + digit;
        bb->offset++;
    }
    if(is_negative){
        value = (-(value));
    }
    *out = value;
    return 1;
}

int64_t bencode_parse_int(byte_buffer* bb){
    int64_t val;
    if(bb == NULL ||bb->data == NULL || bb->data[0] != 'i')
        return -1;

    bb->offset++;

    int8_t res = extract_int_val(bb, 'e', &val);
    if(res == -1){
        // TODO: implement error handling
        return -1;
    }

    return val;
}

byte_slice* bencode_parse_string(byte_buffer* bb){
    uint8_t* dc;
    int64_t out = 0;
    if(bb == NULL || bb->len == 0 || bb->data == NULL || is_numeric_start(bb->data, dc)){
        if(*dc == 1){
            // string length cant be negative
            return NULL;
        }
        return NULL;
    }
    int res = extract_int_val(bb, ':', &out);
    if(res == -1){
        return NULL;
    }
    if(bb->data[bb->offset] != ':')
        return NULL;

    bb->offset++;

    byte_slice* bs = calloc(1, sizeof(byte_slice));
    if(!bs){
        return NULL;
    }
    bs->length = out;
    bs->data = &bb->data[bb->offset];
    bb->offset += bs->length;

    return bs;
}

byte_list* bencode_parse_list(byte_buffer* bb){
    if(bb == NULL || bb->data == NULL || bb->len == 0 ||bb->data[bb->offset] != 'l' )
        return NULL;

    bb->offset++;

    byte_list* bl = calloc(1, sizeof(byte_list));
    if(!bl){
        return NULL;
    }

    // whats the element type ?!

    while(bb->offset <= bb->len && bb->data[bb->offset] != 'e'){
        if(bb->data[bb->offset] == 'i'){
            int64_t val = bencode_parse_int(bb);
            // this is not safe
            // TODO : make better errror handling plan
            if(val == -1){
                return NULL;
            }
            bl->type = BENCODE_INT;
            bl->i = val;

            bl = list_next(bl);
            if(!bl)
                return NULL;

            continue;
        }
        else if(bb->data[bb->offset] == 'l'){
            byte_list* bll = bencode_parse_list(bb); 
            if(bll == NULL){
                return NULL;
            }
            bl->type = BENCODE_LIST;

            bl = list_next(bl);
            if(!bl)
                return NULL;

            continue;
        }
        else if(bb->data[bb->offset] == 'd'){
            byte_dict* bd = bencode_parse_dict(bb);
            if(!bd)
                return NULL;
            bl->type = BENCODE_DICT;
            bl->d = bd;

            bl = list_next(bl);
            if(!bl)
                return NULL;

            continue;
        }
        else if(is_digit(bb)){
            byte_slice* bs = bencode_parse_string(bb);
            if(bs == NULL){
                return NULL;
            }
            bl->type = BENCODE_STRING;

            bl = list_next(bl);
            if(!bl)
                return NULL;

            continue;
        }
        else{
            return NULL;
        }
    }
    bb->offset++;
    return bl;
}

byte_dict* bencode_parse_dict(byte_buffer* bb){
    if(bb == NULL || bb->data == NULL || bb->len == 0 ||bb->data[bb->offset] != 'd' )
        return NULL;
    bb->offset++;

    // we are on the first key now and keys are string types

    byte_dict* bd_head = calloc(1, sizeof(byte_dict));
    if(bd_head == NULL){
        return NULL;
    }
    byte_dict* bd = bd_head;

    while(bb->len > bb->offset && bb->data[bb->offset] != 'e'){
        bd->key = bencode_parse_string(bb);

        if(bb->data[bb->offset] == 'i'){
            int64_t val = bencode_parse_int(bb);
            // this is not safe
            // TODO : make better errror handling plan
            if(val == -1){
                return NULL;
            }
            bd->type_val = BENCODE_INT;
            bd->val.i = val;

            bd = dict_next(bd);
            if(bd == NULL)
                return NULL;

            continue;
        }
        else if(bb->data[bb->offset] == 'l'){
            byte_list* bll = bencode_parse_list(bb); 
            if(bll == NULL){
                return NULL;
            }
            bd->type_val = BENCODE_LIST;
            bd->val.l = bll;
            bd = dict_next(bd);
            if(bd == NULL)
                return NULL;
            continue;
        }
        else if(bb->data[bb->offset] == 'd'){
            byte_dict* bdd = bencode_parse_dict(bb);
            if(bdd == NULL){
                return NULL;
            }
            bd->type_val = BENCODE_DICT;
            bd->val.d = bdd;
            bd = dict_next(bd);
            if(bd == NULL)
                return NULL;
            continue;
        }
        else if (is_digit(bb)) {
            byte_slice *bs = bencode_parse_string(bb);
            if (bs == NULL) {
                return NULL;
            }
            bd->type_val = BENCODE_STRING;
            bd->val.s = *bs;
            bd = dict_next(bd);
            if(bd == NULL)
                return NULL;
            continue;
        }
        else{
            return NULL;
        }
    }
    return bd;
}
