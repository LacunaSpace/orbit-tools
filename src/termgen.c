#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sysexits.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include "geo.h"
#include "version.h"
#include "opt_util.h"
#include "constants.h"
#include "util.h"
#include "debug.h"

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] <LOCATION>\n", executable);
    printf("       %s [OPTION...] <LAT> <LON>\n", executable);
    printf("\n");
    printf("<LOCATION> is the name of a city or country (depending on --cities and\n");
    printf("           --countries options) to use as reference point\n");
    printf("<LAT> and <LON> are the latitude (between -90 and 90, inclusive) and\n");
    printf("          longitude (between -180 and 180, inclusive) of the reference\n");
    printf("          point\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help            : Print this help and exit\n");
    printf("-V,--version         : Print version and exit\n");
    printf("-v,--verbose         : Enable debug logging\n");
    printf("-c,--cities          : Use only cities\n");
    printf("-C,--countries       : Use only countries\n");
    printf("-l,--list            : List cities and countries\n");
    printf("-n,--number=<NUMBER> : Number of terminals to generate. Only used together\n");
    printf("                       with the --radius option. Defaults to 1\n");
    printf("-r,--radius=<RADIUS> : Generate terminals in a circle around the reference\n");
    printf("                       point with the given radius in km\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}

static int find_reference_point_by_name(const char *name, double *lat, double *lon, int only_cities, int only_countries) { 
    if(!only_countries) {
        for(size_t l=0; l<nr_cities; l++) {
            if(!strcasecmp(name, cities[l].name) ||
               !strcasecmp(name, cities[l].name_ascii)) {
                *lat = cities[l].lat;
                *lon = cities[l].lon;
                return 0;
            }
        }
    }

    if(!only_cities) {
        for(size_t l=0; l<nr_countries; l++) {
            if(!strcasecmp(name, countries[l].name) ||
               !strcmp(name, countries[l].country_code)) {
                *lat = countries[l].lat;
                *lon = countries[l].lon;
                return 0;
            }
        }
    }

    return -1;
}

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { "verbose", no_argument, NULL, 'v' },
        { "list", no_argument, NULL, 'l' },
        { "cities", no_argument, NULL, 'c' },
        { "countries", no_argument, NULL, 'C' },
        { "number", required_argument, NULL, 'n' },
        { "radius", required_argument, NULL, 'r' },
        { NULL }
    };

    int list = 0,
        only_cities = 0,
        only_countries = 0,
        number = 1,
        radius = 0;

    opterr = 0;
    int c;
    while((c = getopt_long(argc, argv, "hVvlcCn:r:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'V':
                printf("%s\n", VERSION);
                exit(0);
            case 'v':
                debug_enable(1);
                break;
            case 'l':
                list = 1;
                break;
            case 'c':
                only_cities = 1; only_countries = 0;
                break;
            case 'C':
                only_countries = 1; only_cities = 0;
                break;
            case 'n': 
                if(optarg_as_int(&number, 1, INT_MAX)) 
                    usage_error("Error: invalid --number\n");
                break;
            case 'r':
                if(optarg_as_int(&radius, 1, INT_MAX))
                    usage_error("Error: invalid --radius\n");
                break;
            default: {
                double dummy;
                if(!arg_as_double(argv[optind], &dummy) && dummy < 0) 
                    usage_error("Unknown option, please use -- to separate arguments from options when latitude or longitude is negative");
                else
                    usage_error("Unknown option");
                break;
            }
        }
    }

    if(list) {
        if(!only_countries)
            for(size_t l=0; l<nr_cities; l++)
                printf("%s\n", cities[l].name);
        if(!only_cities)
            for(size_t l=0; l<nr_countries; l++)
                printf("%s [%s]\n", countries[l].name, countries[l].country_code);
        exit(0);
    }

    double lat=0, lon=0;

    switch(argc-optind) {
        case 1:
            if(find_reference_point_by_name(argv[argc-1], &lat, &lon, only_cities, only_countries))
                usage_error("location not found");
            break;
        case 2:
            if(arg_as_double_incl_incl(argv[argc-2], &lat, -90.0, 90))
                usage_error("invalid latitude");
            if(arg_as_double_incl_incl(argv[argc-1], &lon, -180.0, 180))
                usage_error("invalid longitude");
            break;
        default:
            usage_error("specify either location as name, or latitude and longitude");
            break;
    }

    if(radius <= 1) {
        printf("%g,%g\n", lat, lon);
    } else {
        /* First generate a circle with the request radius around the reference-point */
        double alpha_deg = rad_to_deg(radius / WGS84_A);
        DEBUG("alpha_deg=%g", alpha_deg);
        double lat_p = lat + alpha_deg;
        DEBUG("p=(%g, %g)", lat_p, lon);
        double v[3], /* Unit vector pointing to the reference location */
               p[3]; /* Unit vector pointing to the first point at the cirlce around the reference location */
        lat_lon_to_vec3(lat, lon, v);
        lat_lon_to_vec3(lat_p, lon, p);
        DEBUG("reference point: (%g, %g, %g)", v[0], v[1], v[2]);
        DEBUG("point on circle: (%g, %g, %g)", p[0], p[1], p[2]);

        size_t steps = number;
        for(size_t l=0; l<steps; l++) {
            double pp[3], aux[3];
            /* Use Rodrigues formula to rotate p around v */
            double theta = l * 2 * M_PI / steps;
            DEBUG("theta=%g", theta);
            double cos_theta = cos(theta),
                   sin_theta = sin(theta);
            
            vec3_copy(pp, p);
            vec3_scalar_mult(pp, cos_theta, pp);
            DEBUG("pp=(%g, %g, %g)", pp[0], pp[1], pp[2]);

            cross_product(v, p, aux);
            vec3_scalar_mult(aux, sin_theta, aux);
            DEBUG("aux=(%g, %g, %g)", aux[0], aux[1], aux[2]);
            vec3_add_to(pp, aux);
            DEBUG("pp=(%g, %g, %g)", pp[0], pp[1], pp[2]);

            vec3_copy(aux, v);
            vec3_scalar_mult(aux, dot_product(v, p), aux);
            vec3_scalar_mult(aux, 1.0 - cos_theta, aux);
            vec3_add_to(pp, aux);
            DEBUG("pp=(%g, %g, %g)", pp[0], pp[1], pp[2]);

            double lat = rad_to_deg(asin(pp[2]));
            double lon = rad_to_deg(atan2(pp[1], pp[0]));
            DEBUG("Point at (%g, %g, %g), lat %g, lon %g", pp[0], pp[1], pp[2], lat, lon);
            printf("%g,%g\n", lat, lon);
        }
        
    }
}

