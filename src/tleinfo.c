#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sysexits.h>
#include <string.h>
#include <time.h>
#include "tle_loader.h"

static char *executable;

void usage(void) {
    fprintf(stderr, "Usage: %s [OPTION...] <TLE_FILE>\n", executable);
    fprintf(stderr, "\n");
    fprintf(stderr, "TLE_FILE is the name of the TLE-file. It\n");
    fprintf(stderr, "may contain data of one or more satellites. Use\n");
    fprintf(stderr, "- to read from stdin\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "-h,--help                 : show this help and exit\n");
    fprintf(stderr, "-n,--satelite-name=<NAME> : the name of the satellite of\n");
    fprintf(stderr, "                            which to show information. The\n");
    fprintf(stderr, "                            default is top who information about\n");
    fprintf(stderr, "                            all satellites in the file\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}

static void print(char *name, TLE *tle) {
    time_t t = tle->epoch/1000;
    struct tm *fmt = gmtime(&t);
    fprintf(stdout, "Name                                : %s\n", name ? name : "<no name>");
    fprintf(stdout, "Object ID                           : %s\n", tle->objectID);
    fprintf(stdout, "Epoch                               : %04d-%02d-%02dT%02d:%02d:%02dZ\n",
                    fmt->tm_year + 1900, fmt->tm_mon+1, fmt->tm_mday,
                    fmt->tm_hour, fmt->tm_min, fmt->tm_sec);
    fprintf(stdout, "1st derivative of mean motion (ndot): %g\n", tle->ndot);
    fprintf(stdout, "2nd derivative of mean motion       : %g\n", tle->nddot);
    fprintf(stdout, "B*                                  : %g\n", tle->bstar);
    fprintf(stdout, "Inclination                         : %g\n", tle->incDeg);
    fprintf(stdout, "RAAN                                : %g\n", tle->raanDeg);
    fprintf(stdout, "Eccentricity                        : %g\n", tle->ecc);
    fprintf(stdout, "Argument of perigee                 : %g\n", tle->argpDeg);
    fprintf(stdout, "Mean anomaly                        : %g\n", tle->maDeg);
    fprintf(stdout, "Mean motion                         : %g\n", tle->n);
    fprintf(stdout, "Rev num                             : %d\n", tle->revnum);
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
    while((c = getopt_long(argc, argv, "hn:", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'n':
                free(sat_name);
                sat_name = strdup(optarg);
                break;
        }
    }

    if(optind != argc-1) usage_error("Filename missing");

    loaded_tle *lt = load_tles_from_filename(argv[argc-1]);
    if(!lt) usage_error("Failed to load file");

    if(sat_name) {
        loaded_tle *target = get_tle_by_name(lt, sat_name);
        if(!target) {
            unload_tles(lt);
            usage_error("Satellite not found");
        }
        print(target->name, &target->tle);
    } else {
        for(loaded_tle *target = lt; target; target=target->next) {
            print(target->name, &target->tle);
            if(target->next) fprintf(stdout, "----------------------------------------------------------\n");
        }
    }

    unload_tles(lt);

    

    
    
}
