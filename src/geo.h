#ifndef _geo_h_
#define _geo_h_

#include <stddef.h>

typedef struct {
    char *name;
    char *name_ascii;
    double lon, lat;
} city;

extern city cities[];
extern size_t nr_cities;

typedef struct {
    char *name;
    char *country_code;
    double lon, lat;
} country;

extern country countries[];
extern size_t nr_countries;

#endif
