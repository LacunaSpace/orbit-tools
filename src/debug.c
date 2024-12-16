#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static int debug_enabled = 0;

void debug_enable(int enable) {
    debug_enabled = enable;
}

void debug(const char *file, int line, const char *fmt, ...) {
    if(!debug_enabled) 
        return;

    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    fprintf(stderr, "%s:%d %s\n", file, line, buf);
}
