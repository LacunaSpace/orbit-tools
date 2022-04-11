#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sysexits.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include "opt_util.h"
#include "tle_loader.h"
#include "TLE.h"
#include "observer.h"

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] <TLE-FILE>\n", executable);
    printf("\n");
    printf("<TLE-FILE> is a file containing one or more TLEs. Use\n");
    printf("- to read the TLEs from stdin.\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help                      : Print this help and exit\n");
    printf("-l,--location=<LON,LAT>        : Specify the location on the ground, in degrees.\n");
    printf("                                 The default is 0,0.\n");
    printf("-n,--satellite-name=<NAME>     : Find passes for the named satellite. When\n");
    printf("                                 not specified, find passes for all satellites\n");
    printf("                                 in the TLE file.\n");
    printf("-e,--min-elevation=<ELEVATION> : Find only passes with a best elevation of at\n");
    printf("                                 least <ELEVATION> degrees. The default is 0.\n");
    printf("-c,--count=<COUNT>             : Stop after finding <COUNT> passes. The default\n");
    printf("                                 is 1.\n");
    printf("-s,--start=<START>             : Start searching at the specified start-date and\n");
    printf("                                 -time, specified as yyyy-mm-ddThh:mm:ssZ. The\n");
    printf("                                 default is the current date and time\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}

typedef struct {
    char *name;
    TLE *tle;
    int in_pass;
    time_t pass_start;
    double best_elevation;
    double best_azimuth;
} scanner;

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "location", required_argument, NULL, 'l' },
        { "satellite-name", required_argument, NULL, 'n' },
        { "min-elevation", required_argument, NULL, 'e' },
        { "count", required_argument, NULL, 'c' },
        { "start", required_argument, NULL, 's' },
        { NULL }
    };

    opterr = 0;

    int c;

    observer obs;
    obs.alt = 0;
    struct timeval start;
    gettimeofday(&start, 0);
    start.tv_usec = 0;
    int count = 1;
    char *sat_name = NULL;
    int min_elevation = 0;

    while((c = getopt_long(argc, argv, "hl:n:e:c:s:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'l':
                if(optarg_as_lon_lat(&obs.lon, &obs.lat))
                    usage_error("Invalid location");
                break;
            case 'n':
                free(sat_name);
                sat_name = strdup(optarg);
                break;
            case 'e':
                if(optarg_as_int(&min_elevation, 0, 90))
                    usage_error("Invalid elevation");
                break;
            case 'c':
                if(optarg_as_int(&count, 1, INT_MAX))
                    usage_error("Invalid count");
                break;
            case 's':
                if(optarg_as_datetime(&start.tv_sec))
                    usage_error("Invalid start");
                break;
            default:
                usage_error("invalid option");
                break;
        }
    }

    if(optind != argc-1) usage_error("Missing filename");

    loaded_tle *lt = load_tles_from_filename(argv[argc-1]);
    if(!lt) usage_error("Failed to read file");

    int nr_sats;
    scanner *scanners;

    if(sat_name) {
        nr_sats=1;
        loaded_tle *target = get_tle_by_name(lt, sat_name);
        if(!target) {
            unload_tles(lt);
            usage_error("Satellite not found");
        }
        scanners = malloc(sizeof(scanner));
        scanners->name=sat_name;
        scanners->tle=&target->tle;
    } else {
        nr_sats = count_tles(lt);
        scanners = malloc(sizeof(scanner) * nr_sats);
        for(size_t l=0; l<nr_sats; l++) {
            loaded_tle *p = get_tle_by_index(lt, l);
            scanners[l].name = p->name;
            scanners[l].tle = &p->tle;
        }
    }

    while(count) {
        observation result;
        for(size_t l=0; l<nr_sats; l++) {
            observe(&obs, &result, scanners[l].tle, start.tv_sec);
            if(result.elevation >= min_elevation && !scanners[l].in_pass) {
                scanners[l].pass_start = start.tv_sec;
                scanners[l].in_pass = 1;
                scanners[l].best_elevation = result.elevation;
                scanners[l].best_azimuth = result.azimuth;
            } else if(result.elevation < min_elevation && scanners[l].in_pass) {
                scanners[l].in_pass = 0;
                struct tm begin_f, end_f;
                time_t begin = scanners[l].pass_start, end = start.tv_sec-1;
                gmtime_r(&begin, &begin_f);
                gmtime_r(&end, &end_f);
                printf("%04d-%02d-%02dT%02d:%02d:%02dZ ", begin_f.tm_year + 1900, begin_f.tm_mon+1, begin_f.tm_mday, begin_f.tm_hour, begin_f.tm_min, begin_f.tm_sec);
                printf("%04d-%02d-%02dT%02d:%02d:%02dZ ", end_f.tm_year + 1900, end_f.tm_mon+1, end_f.tm_mday, end_f.tm_hour, end_f.tm_min, end_f.tm_sec);
                printf("%s %g %g\n", scanners[l].name, scanners[l].best_elevation, scanners[l].best_azimuth);
                count--;
            } else if(scanners[l].in_pass && result.elevation > scanners[l].best_elevation) {
                scanners[l].best_elevation = result.elevation;
                scanners[l].best_azimuth = result.azimuth;
            }
        }
        start.tv_sec++;
    }
}
