#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sysexits.h>
#include <string.h>
#include "geo.h"

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] <CITY>\n", executable);
    printf("\n");
    printf("<CITY> is the name of a city where to generate a terminal (in\n");
    printf("       other words, whose location to return).\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help: Print this help and exit\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}
int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "--help", no_argument, NULL, 'h' },
        { NULL }
    };

    opterr = 0;
    int c;
    while((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            default:
                usage_error("Unknown option");
                break;
        }
    }
    if(optind != argc-1) usage_error("City-name missing");

    for(size_t l=0; l<nr_cities; l++) {
        if(!strcasecmp(argv[argc-1], cities[l].name) ||
           !strcasecmp(argv[argc-1], cities[l].name_ascii)) {
            printf("%g,%g\n", cities[l].lon, cities[l].lat);
            exit(0);
        }
    }

    fprintf(stderr, "Error: city not found\n");
    exit(EX_DATAERR);
}

