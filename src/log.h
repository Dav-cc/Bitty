#ifndef BITTY_LOG_H
#define BITTY_LOG_H

/*
 * diffrent types of error
 */

typedef enum {
    ERR_NONE = 0,
    ERR_INVALID_ARGUMENT,
    ERR_IO,
    ERR_BENCODE_INVALID,
    ERR_OUT_OF_MEMORY,
} error_code;


/*
 * logging have 4 stages
 * debug: highst stage and all logs shown and used for monitoring almost every thing 
 * error: only errors (critical or non critical) shown 
 * warning and info: tracking every stage in Bitty like diffrent phases and . . .
 */

typedef enum {
    ll_debug,
    ll_error,
    ll_warning,
    ll_info
}log_level;

/*
 * every bug in Bitty most have this fields
 */
typedef struct {
    error_code ecode;
    log_level ll;
    const char* report;
}error_t;


void log_msg(log_level ll, const char* str, ...);

#endif
