#include "observer.h"
#include "debug.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "constants.h"
#include "util.h"

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



void observe(observer *obs, observation *o, TLE *tle, time_t when) {
    /* Rotational axis of the earth pointing north, needed in various places */
    double rot_axis[3] = { 0.0, 0.0, 1.0 };

    /* Get the location of the satellite in ECI. This also gives us the satellite's 
       velocity */
    double sat_eci[3], sat_velocity_eci[3];
    getRVForDate(tle, when * 1000, sat_eci, sat_velocity_eci);

    /* Now first populate all the position-related fields */
    memcpy(o->sat_eci, sat_eci, sizeof sat_eci);

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
    double down[3], east[3];
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

    /* Now populate the velocity-related fields */
    memcpy(o->sat_velocity_eci, sat_velocity_eci, sizeof sat_velocity_eci);
    o->velocity = vec3_len(sat_velocity_eci);

    /* To calculate the ground-track velocity, we project the satellite's velocity (in ECEF)
       on the plane tangential to the SSP (which we already have in ECEF). For a circular orbit,
       the ground-track velocity is equal to the satellite's velocity, but for an elliptical
       orbit it may be different. */
    double sat_velocity_ecef[3];
    eci_to_ecef(sat_velocity_eci, (double)when, sat_velocity_ecef);

    DEBUG("sat_velocity_eci=(%g, %g, %g)", sat_velocity_eci[0], sat_velocity_eci[1], sat_velocity_eci[2]);
    DEBUG("sat_velocity_ecef=(%g, %g, %g)", sat_velocity_ecef[0], sat_velocity_ecef[1], sat_velocity_ecef[2]);

    double ssp_ecef_norm[3];
    vec3_norm(ssp_ecef, ssp_ecef_norm);
    DEBUG("ssp_ecef=(%g, %g, %g)", ssp_ecef[0], ssp_ecef[1], ssp_ecef[2]);
    DEBUG("ssp_ecef_norm=(%g, %g, %g)", ssp_ecef_norm[0], ssp_ecef_norm[1], ssp_ecef_norm[2]);
    double i = dot_product(sat_velocity_ecef, ssp_ecef_norm);
    DEBUG("i=%g", i);
    /* i tells us how much of the velocity vector is parallel with the SSP vector. If
       we subtract this portion from the velocity vector, what remains is the velocity
       in the plane tangential to the SSP. */
    double groundtrack_velocity[] = {
        sat_velocity_ecef[0] - i * ssp_ecef_norm[0],
        sat_velocity_ecef[1] - i * ssp_ecef_norm[1],
        sat_velocity_ecef[2] - i * ssp_ecef_norm[2]
    };
    DEBUG("groundtrack_velocity=(%g, %g, %g)", groundtrack_velocity[0], groundtrack_velocity[1], groundtrack_velocity[2]);
    o->groundtrack_velocity = vec3_len(groundtrack_velocity);

    /* Now calculate the ground-track direction - this is the azimuth of the groundtrack-velocity on
       the plan tangential to the earth surface at the SSP.
       This is done by first calculating the vector pointing "east" from the SSP (which is perpendicular
       to both the SSP and the north pole) */
    double ssp_east[3], ssp_down[3], ssp_north[3];
    cross_product(ssp_ecef, rot_axis, ssp_east);
    vec3_scalar_mult(ssp_ecef, -1.0, ssp_down);
    cross_product(ssp_east, ssp_down, ssp_north);

    double groundtrack_velocity_norm[3], ssp_east_norm[3], ssp_north_norm[3];
    vec3_norm(groundtrack_velocity, groundtrack_velocity_norm);
    vec3_norm(ssp_east, ssp_east_norm);
    vec3_norm(ssp_north, ssp_north_norm);
    DEBUG("groundtrack_velocity_norm=(%g, %g, %g)", groundtrack_velocity_norm[0], groundtrack_velocity_norm[1], groundtrack_velocity_norm[2]);
    DEBUG("ssp_east_norm=(%g, %g, %g)", ssp_east_norm[0], ssp_east_norm[1], ssp_east_norm[2]);
    DEBUG("ssp_north_norm=(%g, %g, %g)", ssp_north_norm[0], ssp_north_norm[1], ssp_north_norm[2]);

    double cos_gt_velo_north = dot_product(groundtrack_velocity_norm, ssp_north_norm);
    double cos_gt_velo_east = dot_product(groundtrack_velocity_norm, ssp_east_norm);
    double angle_gt_velo_north = acos(cos_gt_velo_north);
    double angle_gt_velo_east = acos(cos_gt_velo_east);
    double groundtrack_dir = angle_gt_velo_east < M_PI/2.0 ? angle_gt_velo_north : 2.0 * M_PI - angle_gt_velo_north;
    groundtrack_dir += M_PI;
    if(groundtrack_dir >= M_PI * 2.0)
        groundtrack_dir -= M_PI * 2.0;
    o->groundtrack_direction = rad_to_deg(groundtrack_dir);
    
}
