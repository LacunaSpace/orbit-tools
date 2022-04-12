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
#include "observer.h"

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

    observer obs;
    obs.alt = 0;
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
                if(optarg_as_lon_lat(&obs.lon, &obs.lat))
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
        observation result;
        observe(&obs, &result, tle, start.tv_sec);
        struct tm fmt;
        gmtime_r(&start.tv_sec, &fmt);
        printf("%04d-%02d-%02dT%02d:%02d:%02dZ %g %g %g %g %g %g\n", fmt.tm_year + 1900, fmt.tm_mon+1, fmt.tm_mday, fmt.tm_hour, fmt.tm_min, fmt.tm_sec, result.range, result.elevation, result.azimuth, result.ssp_lon, result.ssp_lat, result.altitude);

        start.tv_sec += interval;
    }

    unload_tles(lt);
}
