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
        fld_type_string
    } type;
} field;

typedef struct {
    union {
        time_t time_value;
        double double_value;
        char *string_value;
    } value;
} field_value;

int check_selector(field *fields, char *selector);

void render(int count, field *fields, field_value *values, char *selector, int rows);

void render_headers(field *fields, char *selector);
#endif
