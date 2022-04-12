#include "output.h"
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

static void find_field(char c, field *fields, field_value *values, field **f, field_value **v) {
    for(; fields->label; fields++, values++) 
        if(fields->c == c) {
            if(f) *f = fields;
            if(v) *v = values;
            return;
        }
}

static int is_field(char c, field *fields) {
    for(; fields->label; fields++) 
        if(fields->c == c) return 1;
    return 0;
}


static void render_cols(int count, field *fields, field_value *values, char *selector) {
    struct tm fmt;
    int first = 1;
    for(; *selector; selector++) {
        field *f;
        field_value *v;
        find_field(*selector, fields, values, &f, &v);
        if(!first)
            printf(" ");
        switch(f->type) {
            case fld_type_time_string:
                gmtime_r(&v->value.time_value, &fmt);
                printf("%04d-%02d-%02dT%02d:%02d:%02dZ", fmt.tm_year + 1900, fmt.tm_mon+1, fmt.tm_mday,
                                                         fmt.tm_hour, fmt.tm_min, fmt.tm_sec);
                break;
            case fld_type_time:
                printf("%lu", v->value.time_value);
                break;
            case fld_type_double:
                printf("%g", v->value.double_value);
                break;
            case fld_type_string:
                printf("%s", v->value.string_value);
                break;
        }
        first = 0;
    }
    printf("\n");
}

static void render_rows(int count, field *fields, field_value *values, char *selector) {
    if(count) printf("------------------------------\n");

    int max = 0;
    for(char *s=selector; *s; s++) {
        field *f;
        find_field(*s, fields, values, &f, NULL);
        if(strlen(f->label) > max) 
            max = strlen(f->label);
    }

    for(char *s=selector; *s; s++) {
        field *f;
        field_value *v;
        find_field(*s, fields, values, &f, &v);
        printf("%*s : ", max, f->label);
        struct tm fmt;
        switch(f->type) {
            case fld_type_time_string:
                gmtime_r(&v->value.time_value, &fmt);
                printf("%04d-%02d-%02dT%02d:%02d:%02dZ", fmt.tm_year + 1900, fmt.tm_mon+1, fmt.tm_mday,
                                                         fmt.tm_hour, fmt.tm_min, fmt.tm_sec);
                break;
            case fld_type_time:
                printf("%lu", v->value.time_value);
                break;
            case fld_type_double:
                printf("%g", v->value.double_value);
                break;
            case fld_type_string:
                printf("%s", v->value.string_value);
                break;
        }
        printf("\n");
    }
}

void render(int count, field *fields, field_value *values, char *selector, int rows) {
    if(rows) render_rows(count, fields, values, selector);
    else render_cols(count, fields, values, selector);
}

void render_headers(field *fields, char *selector) {
    int first = 1;
    for(; *selector; selector++) {
        field *f;
        find_field(*selector, fields, NULL, &f, NULL);
        if(!first) printf(" ");
        printf("%s", f->label_short);
        first = 0;
    }
    printf("\n");

}

int check_selector(field *fields, char *selector) {
    for(; *selector; selector++)
        if(!is_field(*selector, fields)) return -1;

    return 0;
}
