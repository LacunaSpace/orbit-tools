#ifndef TLEDATA_H
#define TLEDATA_H

#include <sys/time.h>

typedef struct {
    int cat_number;
    char classification;
    int launch_year;
    int launch_number;
    char *launch_piece;
    time_t epoch;
    double ballistic_coeff;
    double second_deriv_mean_motion;
    double bstar;
    int element_set_number;
    double inclination;
    double raan;
    double eccentricity;
    double arg_of_perigee;
    double mean_anomaly;
    double mean_motion;
    int revolution_number;
} tledata;

#endif
