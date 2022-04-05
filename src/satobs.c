#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>
#include <sysexits.h>
#include <limits.h>
#include <string.h>
#include "TLE.h"
#include "opt_util.h"
#include "tle_loader.h"

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

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] <TLE-FILE>\n", executable);
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help                  : show this help and exit.\n");
    printf("-l,--location=<LON,LAT>    : specify the location on the ground, in degrees.\n");
    printf("                             The default is 0,0.\n");
    printf("-s,--start=<TIMESTAMP>     : specify the date and time at which to start the,\n");
    printf("                             calculation, formatted as yyyy-mm-ddThh-mm-ssZ.\n");
    printf("                             The default is the current date and time.\n");
    printf("-c,--count=<COUNT>         : specify the number of calculations to perform.\n");
    printf("                             The default is 1.\n");
    printf("-i,--interval=<INTERVAL>   : specify the interval between consecutive\n");
    printf("                             calculations in seconds. The default is 1.\n");
    printf("-n,--satellite-name=<NAME> : in case the TLE file contains data of multiple\n");
    printf("                             satellites, specify the name of the satellite to\n");
    printf("                             be used. The default is to use the first satellite\n");
    printf("                             from the file.\n");
    printf("\n");
    printf("<TLE-FILE> is the path to the TLE file. Use - to read from stdin\n");
    printf("\n");
    printf("The output consists of space-separated values, one for each calculation.\n");
    printf("The values are: datetime, range in km, elevation in degrees\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}

int main(int argc, char *argv[]) {
    executable = argv[0];
    opterr = 0;
    int c;
    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "location", required_argument, NULL, 'l' },
        { "start", required_argument, NULL, 's' },
        { "count", required_argument, NULL, 'c' },
        { "interval", required_argument, NULL, 'i' },
        { "satellite-name", required_argument, NULL, 'n' },
        { NULL }
    };

    double lon=0, lat=0;
    struct timeval start;
    gettimeofday(&start, 0);
    start.tv_usec = 0;
    int count = 1;
    int interval = 1;
    char *satellite_name = NULL;
    

    while((c = getopt_long(argc, argv, "hl:s:c:i:n:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'l':
                if(optarg_as_lon_lat(&lon, &lat))
                    usage_error("Invalid location");          
                break;
            case 's':
                if(optarg_as_datetime(&start.tv_sec))
                    usage_error("Invalid start");
                break;
            case 'c':
                if(optarg_as_int(&count, 1, INT_MAX))
                    usage_error("Invalid count");
                break;
            case 'i':
                if(optarg_as_int(&interval, 1, INT_MAX))
                    usage_error("Invalid interval");
                break;
            case 'n':
                free(satellite_name);
                satellite_name = strdup(optarg);
                break;
            default:
                usage_error("Invalid option");
                break;
        }
    }

    if(optind != argc-1) 
        usage_error("Missing filename");

    loaded_tle *lt = load_tles_from_filename(argv[argc-1]);
    if(!lt) usage_error("Failed to read file");

    loaded_tle *target_tle = get_tle_by_name(lt, satellite_name);
    if(!target_tle) {
        unload_tles(lt);
        usage_error("Satellite not found");
    }

    TLE *tle = &target_tle->tle;

    for(size_t l=0; l<count; l++) {

        double r[3], v[3];
        getRVForDate(tle, start.tv_sec * 1000, r, v);

        double alt = 0;
        double ecef[3];
        lla_to_ecef(lon, lat, alt, ecef);
        double eci[3];
        ecef_to_eci(ecef, (double)start.tv_sec, eci);

        double dir[3] = {
            r[0] - eci[0], r[1] - eci[1], r[2] - eci[2]
        };
        double range = vec3_len(dir);

        double robs = vec3_len(eci);
        double rsat = vec3_len(r);

        double cosa = (robs*robs - rsat*rsat - range*range) / (-2.0 * rsat * range);
        double a = acos(cosa);

        struct tm fmt;
        gmtime_r(&start.tv_sec, &fmt);
        double dotp = dot_product(eci ,r);
        double dotp2 = dot_product(eci, dir);
        double cos_phi = dotp / (vec3_len(r) * vec3_len(eci));
        double phi = acos(cos_phi);
        double psi0 = M_PI - M_PI/2 - phi;
        double psi = M_PI - psi0;
        double elevation = M_PI - a-psi;
        printf("%04d-%02d-%02dT%02d:%02d:%02dZ %g %g\n", fmt.tm_year + 1900, fmt.tm_mon+1, fmt.tm_mday, fmt.tm_hour, fmt.tm_min, fmt.tm_sec, range, rad_to_deg(elevation));

        start.tv_sec += interval;
    }

    unload_tles(lt);
}
