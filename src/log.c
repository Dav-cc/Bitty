#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"

log_level min_level = ll_info ;

static const char *log_level_name(log_level ll)
{
    switch (ll) {
    case ll_debug:   return "DEBUG";
    case ll_info:    return "INFO";
    case ll_warning: return "WARNING";
    case ll_error:   return "ERROR";
    default:          return "UNKNOWN";
    }
}

void log_msg(log_level ll, const char* str, ...){
    if(ll > min_level){
        return;
    }
    va_list args;
    va_start(args, str);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[35];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    printf("[%s] (%s) - ", time_str, log_level_name(ll));
    vprintf(str, args);
    printf("\n");

    va_end(args);
}
