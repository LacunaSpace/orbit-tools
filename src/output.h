#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <sys/time.h>

typedef struct {
    char *label, *label_short;
    char c;
    enum {
        fld_type_time_string,
        fld_type_time, /* Time as number of seconds since epoch */
        fld_type_double,
        fld_type_string,
        fld_type_int
    } type;
} field;

typedef struct {
    union {
        time_t time_value;
        double double_value;
        const char *string_value;
        int int_value;
    } value;
} field_value;

int check_selector(const field *fields, const char *selector);

void render(int count, const field *fields, const field_value *values, const char *selector, int rows);

void render_headers(const field *fields, const char *selector);
#endif
