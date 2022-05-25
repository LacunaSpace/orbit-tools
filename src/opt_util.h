#include <time.h>

/* Assumes optarg contains a lon and lat in degrees, separate by a comma */
int optarg_as_lon_lat(double *lon, double *lat);

/* Assumes optarg contains a timestamp as yyyy-mm-ddThh-mm-ssZ */
int optarg_as_datetime(time_t *t); 

/* Assumes optarg contains a timestamp as yyyy-mm-ddThh-mm-ssZ or just yyyy-mm-dd */
int optarg_as_datetime_extended(time_t *t); 

int optarg_as_int(signed int *i, signed int min, signed int max);

/* Only values > min and < max are accepted */
int optarg_as_double_excl_excl(double *d, double min, double max);

/* Values >= min and < max are accepted */
int optarg_as_double_incl_excl(double *d, double min, double max);
