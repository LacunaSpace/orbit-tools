#include <time.h>

/* Assumes optarg contains a lon and lat in degrees, separate by a comma */
int optarg_as_lon_lat(double *lon, double *lat);

/* Assumes optarg contains a timestamp as yyy-mm-ddThh-mm-ssZ */
int optarg_as_datetime(time_t *t); 

int optarg_as_int(signed int *i, signed int min, signed int max);
