#include "orbitcalc.h"
#include "orbitcalc_commands.h"
#include "debug.h"
#include "opt_util.h"
#include "util.h"
#include <stdio.h>
#include <sysexits.h>
#include <getopt.h>
#include <sys/time.h>
#include <math.h>

#define ECI_TO_ECEF (0)
#define ECEF_TO_ECI (1)

static int coord_convert_command_handler(int argc, char *argv[], void *arg) {
    struct option longopts[] = {
        { "time", required_argument, NULL, 't' },
        { NULL }
    };
    DEBUG("eci_to_ecef");
    for(size_t l=0; l<argc; l++) DEBUG("%zu: %s", l, argv[l]);
    int c;

    /* Defaut to current time - we only use the tv_sec portion of the struct */
    struct timeval t;
    gettimeofday(&t, NULL);
    
    while((c = getopt_long(argc, argv, "+t:", longopts, NULL)) != -1) {
        switch(c) {
            case 't':
                if(optarg_as_datetime_extended(&t.tv_sec)) {
                    fprintf(stderr, "Error: invalid --time\n");
                    return EX_USAGE;
                }
                break;
            default:
                fprintf(stderr, "Error: unknown option\n");
                return EX_USAGE;
        }
    }
    DEBUG("time=%d", t.tv_sec);

    if(optind != argc-1) {
        fprintf(stderr, "Error: expected one argument (X,Y,Z without whitespace)\n");
        return EX_USAGE;
    }

    double src[3], dst[3];
    if(arg_as_vec3(argv[optind], src)) {
        fprintf(stderr, "Error: invalid argument\n");
        return EX_USAGE;
    }
    if(arg == ECI_TO_ECEF) 
        eci_to_ecef(src, (double)t.tv_sec, dst);
    else
        ecef_to_eci(src, (double)t.tv_sec, dst);
    printf("%g,%g,%g\n", dst[0], dst[1], dst[2]);
    return EX_OK;
}

orbitcalc_command eci_to_ecef_command = {
    "eci-to-ecef",
    "Converts a 3-d vector from the ECI into the ECEF coordinate system",
    "Usage: eci-to-ecef [OPTION...] <VECTOR>\n"
    "\n"
    "Options are:\n"
    "-t, --time=<TIME> : Sets the date and time at which to perform the conversion,\n"
    "                    formatted as yyyy-mm-ddThh:mm:ssZ. The default value is the\n"
    "                    current date and time\n"
    "\n"
    "<VECTOR> is specified as an X, Y and Z coordinate seperated by commas, without\n"
    "         whitespace. They are in kilometers. If the X coordinate is negative (and\n"
    "         therefore starts with a '-'), separate options and arguments by '--'\n",
    coord_convert_command_handler,
    (void *)ECI_TO_ECEF
};


orbitcalc_command ecef_to_eci_command = {
    "ecef-to-eci",
    "Converts a 3-d vector from the ECEF into the ECI coordinate system",
    "Usage: ecef-to-eci [OPTION...] <VECTOR>\n"
    "\n"
    "Options are:\n"
    "-t, --time=<TIME> : Sets the date and time at which to perform the conversion,\n"
    "                    formatted as yyyy-mm-ddThh:mm:ssZ. The default value is the\n"
    "                    current date and time\n"
    "\n"
    "<VECTOR> is specified as an X, Y and Z coordinate seperated by commas, without\n"
    "         whitespace. They are in kilometers. If the X coordinate is negative (and\n"
    "         therefore starts with a '-'), separate options and arguments by '--'\n",
    coord_convert_command_handler,
    (void *)ECEF_TO_ECI
};

static double safe_acos(double a) {
    if(a > 1.0) a = 1.0;
    if(a < -1.0) a = -1.0;
    return acos(a);
}
static int azimuth_command_handler(int argc, char *argv[], void *unused) {
    DEBUG("azimuth_command_handler");
    if(argc != 3) {
        fprintf(stderr, "Error: need two arguments\n");
        return EX_USAGE;
    }

    double src_lat, src_lon,
           dst_lat, dst_lon;

    if(arg_as_lon_lat(argv[1], &src_lon, &src_lat)) {
        fprintf(stderr, "Error: invalid SRC\n");
        return EX_USAGE;
    }
    if(arg_as_lon_lat(argv[2], &dst_lon, &dst_lat)) {
        fprintf(stderr, "Error: invalid DST\n");
        return EX_USAGE;
    }

    /* 
     * Strategy is as follows:
     * 1) Convert both SRC and DST to ECI
     * 2) Calculate the difference vector between DST and SRC
     * 3) Project this vector onto the plane tangential to the earth surface at SRC
     * 4) Determine the azimuth of this vector
     */
    double src[3], dst[3], dir[3];
    lat_lon_to_vec3(src_lat, src_lon, src);
    lat_lon_to_vec3(dst_lat, dst_lon, dst);
    dir[0] = dst[0] - src[0];
    dir[1] = dst[1] - src[1];
    dir[2] = dst[2] - src[2];
    DEBUG("src: %g,%g,%g", src[0], src[1], src[2]);
    DEBUG("dst: %g,%g,%g", dst[0], dst[1], dst[2]);
    DEBUG("dir: %g,%g,%g", dir[0], dir[1], dir[2]);

    double perp[3], proj[3];
    vec3_scalar_mult(src, dot_product(dir, src), perp);
    DEBUG("perp: %g,%g,%g", perp[0], perp[1], perp[2]);
    proj[0] = dir[0] - perp[0];
    proj[1] = dir[1] - perp[1];
    proj[2] = dir[2] - perp[2];
    /* proj now contains the projection of the vector from src to dst on the plane
       tangential to the earth surface at src */
    DEBUG("proj: %g,%g,%g", proj[0], proj[1], proj[2]);
    vec3_norm(proj, proj);

    /* Now we have to convert it to an azimuth */

    /* First determine the vector pointing east from src. This is the
       cross product of src and the vector from the earth center to the
       north-pole */
    double east[3], earth_axis[3] = { 0, 0, 1.0 };
    cross_product(earth_axis, src, east);
    DEBUG("east: %g,%g,%g", east[0], east[1], east[2]);

    /* Normalize east, we'll need that later */
    vec3_norm(east, east);
    
    /* Now determine the vector pointing north from src. This is the
       cross product of src and the east vector */
    double north[3];
    cross_product(src, east, north);
    DEBUG("north: %g,%g,%g", north[0], north[1], north[2]);
    
    vec3_norm(north, north);

    double cos_east = dot_product(proj, east),
           cos_north = dot_product(proj, north);
    double angle_north = safe_acos(cos_north),
           angle_east = safe_acos(cos_east);
    double azimuth = angle_east <= M_PI/2.0 ? angle_north : 2.0 * M_PI - angle_north;
    printf("%g\n", rad_to_deg(azimuth));
    
    return EX_OK;
}

orbitcalc_command azimuth_command = {
    "azimuth",
    "Calculates the azimuth from a source location to a destination location",
    "Usage: azimuth <SRC> <DST>\n"
    "\n"
    "Both <SRC> and <DST> are locations on earth as latitude and longitude, separated\n"
    "by a comma (and no whitespace). Latitude and longitude are specified as decimals,\n"
    "with the latitude >= -90 and <= 90, and the longitude >= -180 and <= 180.\n"
    "The azimuth is the compass direction in which DST is seen from SRC.\n",
    azimuth_command_handler,
    NULL
};
