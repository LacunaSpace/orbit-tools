#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG(...) do { debug(__FILE__, __LINE__, __VA_ARGS__); } while(0)

void debug(const char *file, int line, const char *fmt, ...);

void debug_enable(int enable);
#endif
