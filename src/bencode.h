#ifndef BITTY_BENCODE_H
#define BITTY_BENCODE_H

#include <stdint.h>
#include <stdio.h>
#include "buffer.h"

typedef struct byte_list byte_list;
typedef struct byte_dict byte_dict;

typedef enum{
    BENCODE_INT,
    BENCODE_STRING,
    BENCODE_LIST,
    BENCODE_DICT
} bencode_type;


/*
 * this is for holding strings
 */
typedef struct {
    const uint8_t* data;
    size_t length;
} byte_slice;


/*
 * l {element, element, . . . } e  --> each element can be string, integer, list or dict 
 * 
 */
struct byte_list{
    bencode_type type;
    union{
        int64_t i;
        byte_slice s;
        byte_list* l;
        byte_dict* d;
    };
    byte_list* next_element;
};

/*
 * dictionarys in torrent are 4th data type and its like this d { key:value, key:value } e
 * and each key is string but valus can be any data types
 */
struct byte_dict{
    byte_slice* key;

    bencode_type type_val;

    union{
        int64_t i;
        byte_slice s;
        byte_list* l;
        byte_dict* d;
    }val;

    byte_dict *next_kv;
};

int8_t extract_int_val(byte_buffer* bb, uint8_t dlim, int64_t* out);
int64_t bencode_parse_int(byte_buffer* bb);

byte_slice* bencode_parse_string(byte_buffer* bb);
byte_dict* bencode_parse_dict(byte_buffer* bb);

#endif // BITTY_BENCODE_H
