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
#include "util.h"
#include "output.h"

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
    printf("-f,--format=rows|cols      : sets the output format. When not specified, rows is\n");
    printf("                             used when count is 1, otherwise cols\n");
    printf("-H,--headers               : when output format is cols, print a row with headers\n");
    printf("                             as first row\n");
    printf("-F,--fields=<FIELDS>       : specifies the fields to print. <FIELDS> consists of the\n");
    printf("                             following characters:\n");
    printf("                             t: The date and time, formatted\n");
    printf("                             T: The date and time, in seconds since the epoch\n");
    printf("                             r: The range, in kilometers\n");
    printf("                             l: The elevation, in degrees\n");
    printf("                             z: The azimuth, in degrees\n");
    printf("                             o: The longitude of the sub-satellite point, in degrees\n");
    printf("                             a: The latitude of the sub-satellite point, in degrees\n");
    printf("                             A: The satellite altitude, in kilometers\n");
    printf("                             The default is trezoaA when a location is specified,\n");
    printf("                             toaA when no location is specified.\n");
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

static field fields[] = {
    { "Time", "time", 't', fld_type_time_string },
    { "Time", "time", 'T', fld_type_time },
    { "Range", "range", 'r', fld_type_double },
    { "Elevation", "elevation", 'l', fld_type_double },
    { "Azimuth", "azimuth", 'z', fld_type_double },
    { "SSP Longitude", "ssp_lon", 'o', fld_type_double },
    { "SSP Latitude", "ssp_lat", 'a', fld_type_double },
    { "Altitude", "altitude", 'A', fld_type_double },
    { NULL }
};    

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
        { "format", required_argument, NULL, 'f' },
        { "fields", required_argument, NULL, 'F' },
        { "headers", no_argument, NULL, 'H' },
        { NULL }
    };

    observer obs;
    obs.alt = 0;
    int has_location = 0;
    struct timeval start;
    gettimeofday(&start, 0);
    start.tv_usec = 0;
    int count = 1;
    int interval = 1;
    char *satellite_name = NULL;
    enum { fmt_auto, fmt_rows, fmt_cols } fmt = fmt_auto;
    char *selector = NULL;
    int headers = 0;
    while((c = getopt_long(argc, argv, "hl:s:c:i:n:f:F:H", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'l':
                if(optarg_as_lon_lat(&obs.lon, &obs.lat))
                    usage_error("Invalid location");          
                has_location = 1;
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
            case 'f':
                if(string_starts_with("rows", optarg)) fmt = fmt_rows;
                else if(string_starts_with("cols", optarg)) fmt = fmt_cols;
                else usage_error("Invalid format");
                break;
            case 'F':
                if(check_selector(fields, optarg)) 
                    usage_error("Invalid fields-string");
                free(selector);
                selector = strdup(optarg);
                break;
            case 'H':
                headers = 1;
                break;
            default:
                usage_error("Invalid option");
                break;
        }
    }

    if(optind != argc-1) 
        usage_error("Missing filename");

    if(!selector)
        selector = has_location ? "trlzoaA" : "toaA";

    if(fmt == fmt_auto) {
        if(count > 1) fmt = fmt_cols;
        else fmt = fmt_rows;
    }

    loaded_tle *lt = load_tles_from_filename(argv[argc-1]);
    if(!lt) usage_error("Failed to read file");

    loaded_tle *target_tle = get_tle_by_name(lt, satellite_name);
    if(!target_tle) {
        unload_tles(lt);
        usage_error("Satellite not found");
    }

    TLE *tle = &target_tle->tle;

    if(fmt == fmt_cols && headers) render_headers(fields, selector);

    field_value values[sizeof fields/sizeof fields[0] - 1];

    for(size_t l=0; l<count; l++) {
        observation result;
        observe(&obs, &result, tle, start.tv_sec);
        values[0].value.time_value = start.tv_sec;
        values[1].value.time_value = start.tv_sec;
        values[2].value.double_value = result.range;
        values[3].value.double_value = result.elevation;
        values[4].value.double_value = result.azimuth;
        values[5].value.double_value = result.ssp_lon;
        values[6].value.double_value = result.ssp_lat;
        values[7].value.double_value = result.altitude;
        render(l, fields, values, selector, fmt == fmt_rows);

        start.tv_sec += interval;
    }

    unload_tles(lt);
}
