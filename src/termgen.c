#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sysexits.h>
#include <string.h>
#include "geo.h"

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...] <LOCATION>\n", executable);
    printf("\n");
    printf("<LOCATION> is the name of a city or country where to generate a terminal (in\n");
    printf("       other words, whose location to return).\n");
    printf("\n");
    printf("Options are:\n");
    printf("-h,--help        : Print this help and exit\n");
    printf("-c,--cities      : Use only cities\n");
    printf("-C,--countries   : Use only countries\n");
    printf("-l,--list        : List cities and countries\n");
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}
int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "--help", no_argument, NULL, 'h' },
        { "--list", no_argument, NULL, 'l' },
        { "--cities", no_argument, NULL, 'c' },
        { "--countries", no_argument, NULL, 'C' },
        { NULL }
    };

    int list = 0,
        only_cities = 0,
        only_countries = 0;

    opterr = 0;
    int c;
    while((c = getopt_long(argc, argv, "hlcC", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'l':
                list = 1;
                break;
            case 'c':
                only_cities = 1; only_countries = 0;
                break;
            case 'C':
                only_countries = 1; only_cities = 0;
                break;
            default:
                usage_error("Unknown option");
                break;
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

    if(optind != argc-1) usage_error("Location missing");

    for(size_t l=0; l<nr_cities; l++) {
        if(!strcasecmp(argv[argc-1], cities[l].name) ||
           !strcasecmp(argv[argc-1], cities[l].name_ascii)) {
            printf("%g,%g\n", cities[l].lat, cities[l].lon);
            exit(0);
        }
    }

    for(size_t l=0; l<nr_countries; l++) {
        if(!strcasecmp(argv[argc-1], countries[l].name) ||
           !strcmp(argv[argc-1], countries[l].country_code)) {
            printf("%g,%g\n", countries[l].lat, countries[l].lon);
            exit(0);
        }
    }

    fprintf(stderr, "Error: Location not found\n");
    exit(EX_DATAERR);
}

