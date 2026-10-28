#include "buffer.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

byte_buffer* byte_buffer_init(size_t cap, size_t data_len){
    if(cap == 0)
        return NULL;

    byte_buffer* bb = calloc(1, sizeof(byte_buffer));
    if(!bb)
        return NULL;

    // it should be stat.st_size length but for now 2048 is enough
    bb->data = calloc(1, data_len);
    if(!bb)
        return NULL;

    bb->len = data_len;
    bb->cap = cap;
    bb->offset = 0;
    return bb;
}

byte_buffer* byte_buffer_read_file(const char* filepath){
    struct stat st;
    size_t readed = 0;

    int filefd = open(filepath,O_RDONLY);
    if(filefd == -1){
        return NULL;
    }

    int res = fstat(filefd, &st);
    if(res == -1){
        return NULL;
    }

    size_t filesize = st.st_size;

    byte_buffer* bb = byte_buffer_init(4068, filesize);
    if(bb == NULL || bb->data == NULL){
        close(filefd);
        return NULL;
    }

    while((readed = read(filefd, (void*)bb->data + bb->offset, filesize)) != 0){
        bb->offset += readed;
    }

    bb->offset =0;
    close(filefd);

    return bb;
}

void byte_buffer_destroy(byte_buffer* bb){
    free((void*)bb->data);
    bb->data = NULL;
    bb->len = 0;
    bb->offset = 0;
    bb->cap = 0;
    free((void*)bb);
}
