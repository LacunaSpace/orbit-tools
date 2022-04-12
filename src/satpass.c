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
#include "util.h"

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
    printf("-f,--format=rows|cols          : Sets the output format. When not specified, rows\n");
    printf("                                 is used when count is 1, otherwise cols\n");
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
    time_t tca;
    double best_elevation;
    double best_azimuth;
    double start_azimuth;
    double end_azimuth;
} scanner;


static void print_timestamp(time_t t) {
    struct tm f;
    gmtime_r(&t, &f); 
    printf("%04d-%02d-%02dT%02d:%02d:%02dZ", f.tm_year + 1900, f.tm_mon+1, f.tm_mday, f.tm_hour, f.tm_min, f.tm_sec);
}

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "location", required_argument, NULL, 'l' },
        { "satellite-name", required_argument, NULL, 'n' },
        { "min-elevation", required_argument, NULL, 'e' },
        { "count", required_argument, NULL, 'c' },
        { "start", required_argument, NULL, 's' },
        { "format", required_argument, NULL, 'f' },
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

    enum {
        fmt_auto,
        fmt_cols,
        fmt_rows
    } fmt = fmt_auto;

    while((c = getopt_long(argc, argv, "hl:n:e:c:s:f:", longopts, NULL)) != -1) {
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
            case 'f':
                if(string_starts_with("rows", optarg)) fmt = fmt_rows;
                else if(string_starts_with("cols", optarg)) fmt = fmt_cols;
                else usage_error("Invalid format");
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

    if(fmt == fmt_auto) fmt = count > 1 ? fmt_cols : fmt_rows;

    size_t pass_count = 0;
    while(pass_count < count) {
        observation result;
        for(size_t l=0; l<nr_sats; l++) {
            observe(&obs, &result, scanners[l].tle, start.tv_sec);
            if(result.elevation >= min_elevation && !scanners[l].in_pass) {
                scanners[l].pass_start = start.tv_sec;
                scanners[l].in_pass = 1;
                scanners[l].best_elevation = result.elevation;
                scanners[l].best_azimuth = result.azimuth;
                scanners[l].tca = start.tv_sec;
                scanners[l].start_azimuth = result.azimuth;
            } else if(result.elevation < min_elevation && scanners[l].in_pass) {
                scanners[l].in_pass = 0;
                time_t begin = scanners[l].pass_start, end = start.tv_sec-1;
                if(fmt == fmt_cols) {
                    print_timestamp(begin); printf(" ");
                    print_timestamp(end); printf(" ");
                    printf("%s %g %g\n", scanners[l].name, scanners[l].best_elevation, scanners[l].best_azimuth);
                } else {
                    if(pass_count) printf("------------------------------------------------\n");
                    printf("AOS         : "); print_timestamp(begin); printf("\n");
                    printf("TCA         : "); print_timestamp(scanners[l].tca); printf("\n");
                    printf("LOS         : "); print_timestamp(end); printf("\n");
                    printf("Duration    : %lds\n", start.tv_sec - scanners[l].pass_start);
                    printf("Elevation   : %g\n", scanners[l].best_elevation);
                    printf("AOS azimuth : %g\n", scanners[l].start_azimuth);
                    printf("TCA azimuth : %g\n", scanners[l].best_azimuth);
                    printf("LOS azimuth : %g\n", result.azimuth);
                    printf("Satellite   : %s\n", scanners[l].name);
                }
                pass_count++;
            } else if(scanners[l].in_pass && result.elevation > scanners[l].best_elevation) {
                scanners[l].best_elevation = result.elevation;
                scanners[l].best_azimuth = result.azimuth;
                scanners[l].tca = start.tv_sec;
            }
        }
        start.tv_sec++;
    }
}
