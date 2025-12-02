#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "utils.h"

void log_msg(const char *level, const char *fmt, ...) {
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", &tm_now);

    fprintf(stdout, "[%s] %s ", level, ts);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    fprintf(stdout, "\n");
    fflush(stdout);
}

void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len-1] == '\n') s[len-1] = '\0';
}
