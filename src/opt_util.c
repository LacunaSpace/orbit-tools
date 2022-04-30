#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
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
    fprintf(stderr, "Invalid: [%s]\n", optarg);
    return -1;
}

int optarg_as_int(signed int *i, signed int min, signed int max) {
    char *p;
    *i = strtol(optarg, &p, 10);
    if(*p || *i < min || *i > max) return -1;
    return 0;
}
