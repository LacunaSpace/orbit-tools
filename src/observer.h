#ifndef _observer_h_
#define _observer_h_

#include "TLE.h"
#include <sys/time.h>
#include <stddef.h>

typedef struct {
    double lon, lat, alt;
} observer;

typedef struct {
    double range;
    double elevation;
    double azimuth;
    double ssp_lon, ssp_lat;
    double altitude;
    double sat_eci[3];
    double velocity;
    double sat_velocity_eci[3];
} observation;

void observe(observer *obs, observation *o, TLE *tle, time_t when);

#endif
