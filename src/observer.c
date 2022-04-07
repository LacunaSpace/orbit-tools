#include "observer.h"
#include <math.h>

#define WGS84_A (6378.137) /* In km */
#define WGS84_E_SQUARED (6.69437999014E-3)
#define WGS84_OMEGA (7.2921159E-5) /* Earth angular velocity in rad/s */

#define J2000 (946728000.0) /* J2000 is the epoch at 1/1/2000 12:00 noon. This constant is the epoch
                               in UNIX seconds */
#define EARTH_ANGLE_AT_J2000 (280.46) /* The rotational angle of the earth at J2000, in degrees */

static double deg_to_rad(double a_deg) {
    return a_deg * (double)M_PI/180.0;
}

static double rad_to_deg(double a_rad) {
    return a_rad * 180.0 / (double)M_PI;
}

static double calc_n(double lat) {
    double sin_lat = sin(lat);

    return WGS84_A / sqrt(1.0 - WGS84_E_SQUARED * sin_lat * sin_lat);
}

static void lla_to_ecef(double lon_deg, double lat_deg, double alt, double ecef[3]) {
    double lat = deg_to_rad(lat_deg),
           lon = deg_to_rad(lon_deg);

    double n = calc_n(lat);

    double x = (n + alt) * cos(lat) * cos(lon);
    double y = (n + alt) * cos(lat) * sin(lon);
    double b_sq_over_a_sq = (1-WGS84_E_SQUARED);
    double z = (b_sq_over_a_sq * n + alt) * sin(lat);

    ecef[0] = x;
    ecef[1] = y;
    ecef[2] = z;
}

/* time is number of seconds since 1/1/1970 */
static void ecef_to_eci(double ecef[3], double time, double eci[3]) {
    double delta_t = time - J2000;
    double a = WGS84_OMEGA * delta_t + deg_to_rad(EARTH_ANGLE_AT_J2000);

    eci[0] = ecef[0] * cos(a) - ecef[1] * sin(a);
    eci[1] = ecef[0] * sin(a) + ecef[1] * cos(a);
    eci[2] = ecef[2];
}

static double vec3_len(double vec[3]) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

static double dot_product(double a[3], double b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void observe(observer *obs, observation *o, TLE *tle, time_t when) {
    double r[3], v[3];
    getRVForDate(tle, when * 1000, r, v);

    double ecef[3];
    lla_to_ecef(obs->lon, obs->lat, obs->alt, ecef);
    double eci[3];
    ecef_to_eci(ecef, (double)when, eci);

    double dir[3] = {
        r[0] - eci[0], r[1] - eci[1], r[2] - eci[2]
    };
    double range = vec3_len(dir);

    double robs = vec3_len(eci);
    double rsat = vec3_len(r);

    double cosa = (robs*robs - rsat*rsat - range*range) / (-2.0 * rsat * range);
    double a = acos(cosa);

    double dotp = dot_product(eci ,r);
    double cos_phi = dotp / (vec3_len(r) * vec3_len(eci));
    double phi = acos(cos_phi);
    double psi0 = M_PI - M_PI/2 - phi;
    double psi = M_PI - psi0;
    double elevation = M_PI - a-psi;

    o->range = range;
    o->elevation = rad_to_deg(elevation);
}
