#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sysexits.h>
#include <string.h>
#include <time.h>
#include "tle_loader.h"
#include "output.h"
#include "util.h"

#define DEFAULT_SELECTOR "nie12BIRxpamN"

static char *executable;

void usage(void) {
    printf("Usage: %s [OPTION...] [<TLE_FILE>]\n", executable);
    printf("\n");
    printf("TLE_FILE is the name of the TLE-file. It\n");
    printf("may contain data of one or more satellites. Use\n");
    printf("- to read from stdin. When not supplied, $ORBIT_TOOLS_TLE\n");
    printf("is consulted for the filename\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help                 : show this help and exit\n");
    printf("-n,--satelite-name=<NAME> : the name of the satellite of\n");
    printf("                            which to show information. The\n");
    printf("                            default is to show information about\n");
    printf("                            all satellites in the file\n");
    printf("-f,--format=FORMAT        : Set the format to either `rows` or\n");
    printf("                            `cols`. The default is `rows`.\n");
    printf("-H,--headers              : When the format is cols, first print\n");
    printf("                            a row with headers.\n");
    printf("-F,--fields=FIELDS        : Specifies the fields to include in the\n");
    printf("                            outputs. FIELDS is a string consisting\n");
    printf("                            of:\n");
    printf("                            n: Name of the satellite\n");
    printf("                            i: Object ID of the satellite\n");
    printf("                            e: TLE epoch, as yyyy-mm-ddThh:mm:ssZ\n");
    printf("                            E: TLE epoch, as seconds since UNIX epoch\n");
    printf("                            1: First derivative of mean motion (ndot)\n");
    printf("                            2: Second derivative of mean motion\n");
    printf("                            B: B*\n");
    printf("                            I: Inclination\n");
    printf("                            R: RAAN\n");
    printf("                            x: Eccentricity\n");
    printf("                            p: Argument of perigee\n");
    printf("                            a: Mean anomaly\n");
    printf("                            m: Mean motion\n");
    printf("                            N: Rev number\n");
    printf("                            The default is %s\n", DEFAULT_SELECTOR);
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}


static field fields[] = {
    { "Name", "name", 'n', fld_type_string },
    { "Object ID", "object_id", 'i', fld_type_string },
    { "Epoch", "epoch", 'e', fld_type_time_string },
    { "Epoch", "epoch", 'E', fld_type_time },
    { "First derivative of mean motion (n dot)", "1st_deriv_mean_motion", '1', fld_type_double },
    { "Second derivative of mean motion", "2nd_deriv_mean_motion", '2', fld_type_double },
    { "B*", "b_star", 'B', fld_type_double },
    { "Inclination", "inclination", 'I', fld_type_double },
    { "RAAN", "raan", 'R', fld_type_double },
    { "Eccentricity", "eccentricity", 'x', fld_type_double },
    { "Argument of perigee", "arg_of_perigee", 'p', fld_type_double },
    { "Mean anomaly", "mean_anomaly", 'a', fld_type_double },
    { "Mean motion", "mean_motion", 'm', fld_type_double },
    { "Rev number", "rev_number", 'N', fld_type_int },
    { NULL }
};

static void print(int x, const char *name, const TLE *tle, const char *selector, int rows) {
    field_value values[sizeof fields / sizeof fields[0] - 1];
    values[0].value.string_value = name ? name : "<no name>";
    values[1].value.string_value = tle->objectID;
    values[2].value.time_value = tle->epoch/1000;
    values[3].value.time_value = tle->epoch/1000;
    values[4].value.double_value = tle->ndot;
    values[5].value.double_value = tle->nddot;
    values[6].value.double_value = tle->bstar;
    values[7].value.double_value = tle->incDeg;
    values[8].value.double_value = tle->raanDeg;
    values[9].value.double_value = tle->ecc;
    values[10].value.double_value = tle->argpDeg;
    values[11].value.double_value = tle->maDeg;
    values[12].value.double_value = tle->n;
    values[13].value.int_value = tle->revnum;
    render(x, fields, values, selector, rows);
}

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "satellite-name", required_argument, NULL, 'n' },
        { NULL }
    };

    opterr = 0;
    int c;
    char *sat_name = NULL;
    int headers = 0;
    int rows = 1;
    char *selector = NULL;
    
    while((c = getopt_long(argc, argv, "hn:f:HF:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'n':
                free(sat_name);
                sat_name = strdup(optarg);
                break;
            case 'H':
                headers = 1;
                break;
            case 'f':
                if(string_starts_with("cols", optarg)) rows = 0;
                else if(string_starts_with("rows", optarg)) rows = 1;
                else usage_error("Invalid format");
                break;
            case 'F':
                if(check_selector(fields, optarg))
                    usage_error("Invalid fields-string");
                free(selector);
                selector = strdup(optarg);
                break;
            default:
                usage_error("Invalid option");
                break;
        }
    }

    char *file = NULL;
    if(optind == argc-1) file=argv[argc-1];
    else if(optind == argc && getenv("ORBIT_TOOLS_TLE")) file = getenv("ORBIT_TOOLS_TLE");
    else usage_error("either supply a filename or set $ORBIT_TOOLS_TLE");

    if(!selector) selector = DEFAULT_SELECTOR;

    loaded_tle *lt = load_tles_from_filename(file);
    if(!lt) usage_error("Failed to load file");

    if(!rows && headers) render_headers(fields, selector);

    if(sat_name) {
        loaded_tle *target = get_tle_by_name(lt, sat_name);
        if(!target) {
            unload_tles(lt);
            usage_error("Satellite not found");
        }
        print(0, target->name, &target->tle, selector, rows);
    } else {
        int l=0;
        for(loaded_tle *target = lt; target; target=target->next) {
            print(l++, target->name, &target->tle, selector, rows);
        }
    }

    unload_tles(lt);

    

    
    
}
