#define _XOPEN_SOURCE /* for strptime */
#define _DEFAULT_SOURCE /* for timegm */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include "opt_util.h"

int optarg_as_lon_lat(double *lon, double *lat) {
    char *p;
    *lat = strtod(optarg, &p);
    if(*p != ',' || *lat < -90.0 || *lat > 90.0) return -1;

    *lon = strtod(++p, &p);
    if(*p || *lon < -180.0 || *lon > 180.0) return -1;

    return 0;
}

int optarg_as_datetime(time_t *t) {
    struct tm time0;
    char *p = strptime(optarg, "%Y-%m-%dT%H:%M:%SZ", &time0);
    if(p && !*p) {
        *t = timegm(&time0);
        return 0;
    }
    return -1;
}

int optarg_as_datetime_extended(time_t *t) {
    struct tm time0;
    memset(&time0, 0, sizeof time0);
    char *p = strptime(optarg, "%Y-%m-%dT%H:%M:%SZ", &time0);
    if(!p || *p) p = strptime(optarg, "%Y-%m-%d", &time0);
    if(p && !*p) {
        *t = timegm(&time0);
        return 0;
    }
    return -1;
}

int optarg_as_int(signed int *i, signed int min, signed int max) {
    char *p;
    *i = strtol(optarg, &p, 10);
    if(*p || *i < min || *i > max) return -1;
    return 0;
}

int optarg_as_double_excl_excl(double *d, double min, double max) {
    return arg_as_double_excl_excl(optarg, d, min, min);
}

int optarg_as_double_incl_excl(double *d, double min, double max) {
    return arg_as_double_incl_excl(optarg, d, min, min);
}

int optarg_as_double_incl_incl(double *d, double min, double max) {
    return arg_as_double_incl_incl(optarg, d, min, min);
}

int arg_as_double_excl_excl(const char *arg, double *d, double min, double max) {
    char *p;
    *d = strtod(arg, &p);
    if(*p || *d <= min || *d >= max) return -1;
    return 0;
}

int arg_as_double_incl_excl(const char *arg, double *d, double min, double max) {
    char *p;
    *d = strtod(arg, &p);
    if(*p || *d < min || *d >= max) return -1;
    return 0;
}

int arg_as_double_incl_incl(const char *arg, double *d, double min, double max) {
    char *p;
    *d = strtod(arg, &p);
    if(*p || *d < min || *d > max) return -1;
    return 0;
}

int arg_as_double(const char *arg, double *d) {
    char *p;
    *d = strtod(arg, &p);
    if(*p) return -1;
    return 0;
}
