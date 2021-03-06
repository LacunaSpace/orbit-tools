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

static double get_earth_rotation(double time) {
    double delta_t = time - J2000;
    return WGS84_OMEGA * delta_t + deg_to_rad(EARTH_ANGLE_AT_J2000);
}

/* time is number of seconds since 1/1/1970 */
static void ecef_to_eci(double ecef[3], double time, double eci[3]) {
    double a = get_earth_rotation(time);

    eci[0] = ecef[0] * cos(a) - ecef[1] * sin(a);
    eci[1] = ecef[0] * sin(a) + ecef[1] * cos(a);
    eci[2] = ecef[2];
}

static void eci_to_ecef(double eci[3], double time, double ecef[3]) {
    double a = -get_earth_rotation(time);

    ecef[0] = eci[0] * cos(a) - eci[1] * sin(a);
    ecef[1] = eci[0] * sin(a) + eci[1] * cos(a);
    ecef[2] = eci[2];
}

static double vec3_len(double vec[3]) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

static void vec3_scalar_mult(double original[3], double scalar, double result[3]) {
    for(size_t l=0; l<3; l++)
        result[l] = original[l] * scalar;
}

static void vec3_norm(double original[3], double result[3]) {
    double len = vec3_len(original);
    vec3_scalar_mult(original, 1.0/len, result);
}

static double dot_product(double a[3], double b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void cross_product(double a[3], double b[3], double result[3]) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2];
    result[2] = a[0]*b[1] - a[1]*b[0];
}

void observe(observer *obs, observation *o, TLE *tle, time_t when) {
    /* Get the location of the satellite in ECI. This also gives us the satellite's 
       velocity, which we don't use for now */
    double sat_eci[3], v[3];
    getRVForDate(tle, when * 1000, sat_eci, v);

    /* We have the observer's location in lon/lat/alt, convert this first
       to ECEF then to ECI */
    double obs_ecef[3];
    lla_to_ecef(obs->lon, obs->lat, obs->alt, obs_ecef);
    double obs_eci[3];
    ecef_to_eci(obs_ecef, (double)when, obs_eci);

    /* Calculate dir, the vector pointing from the observer to the satellite, and
       its length, range */
    double dir[3] = {
        sat_eci[0] - obs_eci[0], sat_eci[1] - obs_eci[1], sat_eci[2] - obs_eci[2]
    };
    double range = vec3_len(dir);

    /* Also calculate the lengths of the obs_eci and sat_eci vectors, so we
       have the lengths of all three sides of a triangle (see calc1.png) */
    double robs = vec3_len(obs_eci);
    double rsat = vec3_len(sat_eci);

    /* Use the cosine rule to calculate alpha (see calc1.png) */
    double cosa = (robs*robs - rsat*rsat - range*range) / (-2.0 * rsat * range);
    double a = acos(cosa);

    /* Use the dot-product of sat_eci and obs_eci to calculate phi (see calc1.png) */
    double dotp = dot_product(obs_eci, sat_eci);
    double cos_phi = dotp / (vec3_len(sat_eci) * vec3_len(obs_eci));
    double phi = acos(cos_phi);

    /* Now we have two angles of the triangle - the third angle is the elevation + 90 degrees */
    double elevation = M_PI - a - phi - M_PI/2.0;

    o->range = range;
    o->elevation = rad_to_deg(elevation);

    /* Next order of business is to calculate the azimuth. To do this we first
       decompose the dir vector in a component along obs_eci and a component perpendicular
       to that. The perpendicular component will be in the plane tangent to 
       the earth surface and touching the earth at the observer's location. */
    double obs_eci_norm[3], dir_norm[3];
    vec3_norm(obs_eci, obs_eci_norm);
    vec3_norm(dir, dir_norm);
    double dir_parallel[3], dir_parallel_norm[3]; /* This will the component of dir parallel to obs_eci */
    vec3_scalar_mult(obs_eci_norm, dot_product(dir, obs_eci_norm), dir_parallel);
    vec3_norm(dir_parallel, dir_parallel_norm);
    /* The perpendicular component is simply the difference between dir and dir_parallel */
    double dir_perp[3] = { dir[0] - dir_parallel[0], dir[1] - dir_parallel[1], dir[2] - dir_parallel[2] };
    double dir_perp_norm[3];
    vec3_norm(dir_perp, dir_perp_norm);

    /* To complete the azimuth calculation we need a vector in the same tangential plane, but
       pointing north. The angle between the two vectors will be the azimuth. To calculate this
       vector, we first compute a vector pointing east from the observer. This vector is 
       perpendicular to both obs_eci and to the earth rotational axis, so we can get it
       using the cross product */
    double down[3], rot_axis[3] = { 0, 0, 1.0 }, east[3];
    vec3_scalar_mult(obs_eci_norm, -1.0, down);
    cross_product(down, rot_axis, east);
    double east_norm[3];
    vec3_norm(east, east_norm);
    /* Now the vector pointing north is the cross product of obs_eci and east */
    double north[3], north_norm[3];
    cross_product(east, down, north);
    vec3_norm(north, north_norm);
    
    /* Since just the angle with the 'north' vector only gives as partial information (an
       angle 0..PI), we'll look at the angle with the 'east' vector as well */
    double cos_az_north = dot_product(north_norm, dir_perp_norm);
    double cos_az_east = dot_product(east_norm, dir_perp_norm);
    double angle_az_north = acos(cos_az_north);
    double angle_az_east = acos(cos_az_east);

    double azimuth = angle_az_east < M_PI/2.0 ? angle_az_north : 2.0 * M_PI - angle_az_north;
    o->azimuth = rad_to_deg(azimuth);

    /* Now we calculate the longitude and latitude of the SSP (sub-satellite point). For this,
       we act as if the earth is a perfect sphere with radius 1, this will yield the correct
       lon and lat */
    double sat_ecef[3], sat_ecef_norm[3];
    eci_to_ecef(sat_eci, when, sat_ecef);
    vec3_norm(sat_ecef, sat_ecef_norm);

    double ssp_lat = asin(sat_ecef_norm[2]);
    double ssp_lon = atan2(sat_ecef_norm[1], sat_ecef_norm[0]);
    o->ssp_lon = rad_to_deg(ssp_lon);
    o->ssp_lat = rad_to_deg(ssp_lat);

    /* Finally to calculate the satellite altitude, we'll transform ssp_lon/ssp_lat to ecef (using
       the WGS84 ellipsoid) at altitude 0, and simply subtract the length of the vector from sat_ecef */
    double ssp_ecef[3];
    lla_to_ecef(o->ssp_lon, o->ssp_lat, 0, ssp_ecef);
    o->altitude = vec3_len(sat_ecef) - vec3_len(ssp_ecef);
}
