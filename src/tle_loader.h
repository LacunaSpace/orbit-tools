#include <stdio.h>
#include "TLE.h"

typedef struct loaded_tle_t {
    char *name; /* May be NULL */
    TLE tle;
    struct loaded_tle_t *next;
} loaded_tle;

loaded_tle *load_tles(FILE *in);

loaded_tle *load_tles_from_filename(char *filename);

loaded_tle *get_tle_by_name(loaded_tle *lt, char *name);

void unload_tles(loaded_tle *lt);




