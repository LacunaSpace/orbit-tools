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
#include "output.h"
#include "version.h"

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] [<TLE-FILE>]\n", executable);
    printf("\n");
    printf("<TLE-FILE> is a file containing one or more TLEs. Use\n");
    printf("- to read the TLEs from stdin. If <TLE-FILE> is not supplied\n");
    printf("then $ORBIT_TOOLS_TLE must be set to the filename to be used\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help                      : Print this help and exit\n");
    printf("-V,--version                   : Print version and exit\n");
    printf("-l,--location=<LAT,LON>        : Specify the location on the ground, in degrees.\n");
    printf("                                 The default is 0,0.\n");
    printf("-n,--satellite-name=<NAME>     : Find passes for the named satellite. When\n");
    printf("                                 not specified, find passes for all satellites\n");
    printf("                                 in the TLE file.\n");
    printf("-e,--min-elevation=<ELEVATION> : Find only passes with a best elevation of at\n");
    printf("                                 least <ELEVATION> degrees. The default is 0.\n");
    printf("-c,--count=<COUNT>             : Stop after finding <COUNT> passes. The default\n");
    printf("                                 is 1 if no end-date is specified with --end,\n");
    printf("                                 otherwise there is no limit.\n");
    printf("-s,--start=<START>             : Start searching at the specified start-date and\n");
    printf("                                 -time, specified as yyyy-mm-ddThh:mm:ssZ. The\n");
    printf("                                 default is the current date and time. Alternatively\n");
    printf("                                 the start-date can be specified as just yyyy-mm-dd,\n");
    printf("                                 which is short for yyyy-mm-ddT00:00:00Z\n");
    printf("-E,--end=<END>                 : Stop searching at the specified end-date and -time,\n");
    printf("                                 specified as yyyy-mm-ddThh:mm:ssZ. By default, there\n");
    printf("                                 is no end-date, and the number of passes is determined\n");
    printf("                                 but the --count option.\n");
    printf("-f,--format=rows|cols          : Sets the output format. When not specified, rows\n");
    printf("                                 is used when count is 1, otherwise cols\n");
    printf("-F,--fields=<FIELDS>           : Specifies the fields to include in the output.\n");
    printf("                                 <FIELDS> is a string consisting of:\n");
    printf("                                 s: The pass start time, formatted\n");
    printf("                                 S: The pass start time, in seconds since the epoch\n");
    printf("                                 e: The pass end time, formatted\n");
    printf("                                 E: The pass end time, in seconds since the epoch\n");
    printf("                                 t: The time of closest approach, formatted\n"); 
    printf("                                 T: The time of closest approach, in seconds since the epoch\n");
    printf("                                 d: The pass duration, in seconds\n");
    printf("                                 l: The highest elevation, in degrees\n");
    printf("                                 n: The name of the satellite\n");
    printf("                                 z: The azimuth when the elevation is the highest\n");
    printf("                                 Z: The azimuth at the start of the pass\n");
    printf("                                 Y: The azimuth at the end of the pass\n");
    printf("                                 The default is ndstel\n");
    printf("-H,--headers                   : When the format is cols, first print a row with headers\n");
    printf("-g,--give-up-after=<HOURS>     : When no pass found after <HOURS> hours, give up with an\n");
    printf("                                 error. The default is 168 hours, or one week.\n");
    
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
} scanner;

static field fields[] = {
    { "Pass start", "pass_start", 's', fld_type_time_string },
    { "Pass start", "pass_start", 'S', fld_type_time },
    { "Pass end", "pass_end", 'e', fld_type_time_string },
    { "Pass end", "pass_end", 'E', fld_type_time },
    { "Time of closest approach", "tca", 't', fld_type_time_string },
    { "Time of closest approach", "tca", 'T', fld_type_time },
    { "Duration", "duration", 'd', fld_type_time },
    { "Best elevation", "elevation", 'l', fld_type_double },
    { "Satellite", "satellite", 'n', fld_type_string },
    { "TCA azimuth", "tca_azimuth", 'z', fld_type_double },
    { "Start azimuth", "start_azimuth", 'Z', fld_type_double },
    { "End azimuth", "end_azimuth", 'Y', fld_type_double },
    { NULL }
};

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { "location", required_argument, NULL, 'l' },
        { "satellite-name", required_argument, NULL, 'n' },
        { "min-elevation", required_argument, NULL, 'e' },
        { "count", required_argument, NULL, 'c' },
        { "start", required_argument, NULL, 's' },
        { "end", required_argument, NULL, 'E' },
        { "format", required_argument, NULL, 'f' },
        { "fields", required_argument, NULL, 'F' },
        { "headers", no_argument, NULL, 'H' },
        { "give-up-after", required_argument, NULL, 'g' },
        { NULL }
    };

    opterr = 0;

    int c;

    observer obs;
    obs.alt = 0;
    struct timeval start;
    gettimeofday(&start, 0);
    start.tv_usec = 0;
    struct timeval end;
    end.tv_usec = 0;
    int has_end = 0;
    int count = 1;
    int has_count = 0;
    char *sat_name = NULL;
    int min_elevation = 0;
    char *selector = NULL;
    int headers = 0;
    int give_up_after = 7 * 24;

    enum {
        fmt_auto,
        fmt_cols,
        fmt_rows
    } fmt = fmt_auto;

    while((c = getopt_long(argc, argv, "hVl:n:e:c:s:E:f:F:Hg:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'V':
                printf("%s\n", VERSION);
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
                has_count = 1;
                break;
            case 's':
                if(optarg_as_datetime_extended(&start.tv_sec))
                    usage_error("Invalid start");
                break;
            case 'E':
                if(optarg_as_datetime_extended(&end.tv_sec))
                    usage_error("Invalid end");
                has_end = 1;
                break;
            case 'f':
                if(string_starts_with("rows", optarg)) fmt = fmt_rows;
                else if(string_starts_with("cols", optarg)) fmt = fmt_cols;
                else usage_error("Invalid format");
                break;
            case 'F':
                if(check_selector(fields, optarg))
                    usage_error("Invalid fields-string");
                selector = strdup(optarg);
                break;
            case 'H':
                headers = 1;
                break;
            case 'g':
                if(optarg_as_int(&give_up_after, 1, INT_MAX))
                    usage_error("Invalid give-up-after");
                break;
            default:
                usage_error("invalid option");
                break;
        }
    }

    char *file = NULL;
    if(optind == argc-1) file = argv[argc-1];
    else if(optind == argc && getenv("ORBIT_TOOLS_TLE")) file = getenv("ORBIT_TOOLS_TLE");
    else usage_error("Supply a filename or set ORBIT_TOOLS_TLE");

    loaded_tle *lt = load_tles_from_filename(file);
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
    for(size_t l=0; l<nr_sats; l++) {
        scanners[l].in_pass = 0;
    }

    if(fmt == fmt_auto) fmt = has_count | has_end ? fmt_cols : fmt_rows;

    if(!selector) selector = "ndstel";

    field_value values[sizeof fields/sizeof fields[0] - 1 ];

    if(fmt == fmt_cols && headers) render_headers(fields, selector);

    size_t pass_count = 0;
    long long int keep_going = give_up_after * 60 * 60;
    while((pass_count < count || (has_end && !has_count)) && 
          (!has_end || start.tv_sec < end.tv_sec) &&
           keep_going) {
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
                values[0].value.time_value = begin;
                values[1].value.time_value = begin;
                values[2].value.time_value = end;
                values[3].value.time_value = end;
                values[4].value.time_value = scanners[l].tca;
                values[5].value.time_value = scanners[l].tca;
                values[6].value.time_value = end - begin;
                values[7].value.double_value = scanners[l].best_elevation;
                values[8].value.string_value = scanners[l].name ? scanners[l].name : "unknown";
                values[9].value.double_value = scanners[l].best_azimuth;
                values[10].value.double_value = scanners[l].start_azimuth;
                values[11].value.double_value = result.azimuth;
                render(pass_count, fields, values, selector, fmt == fmt_rows);
                pass_count++;
                keep_going = give_up_after * 60 * 60;
            } else if(scanners[l].in_pass && result.elevation > scanners[l].best_elevation) {
                scanners[l].best_elevation = result.elevation;
                scanners[l].best_azimuth = result.azimuth;
                scanners[l].tca = start.tv_sec;
            }
        }
        start.tv_sec++;
        keep_going--;
    }

    if(!keep_going) {
        fprintf(stderr, "No more passes found within %d hours. Consider increasing --give-up-after\n",
                        give_up_after);
        exit(EX_UNAVAILABLE);
    }
}
