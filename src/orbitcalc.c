#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <getopt.h>
#include <string.h>
#include "orbitcalc.h"
#include "orbitcalc_commands.h"
#include "debug.h"

#ifndef VERSION
#   define VERSION "Unknown"
#endif

static int help_command_handler(int argc, char *argv[], void *unused);

orbitcalc_command help_command = {
    "help",
    "Display help about a specific command",
    "Usage: help <COMMAND>\n"
    "\n"
    "<COMMAND> is the command for which to display help\n",
    help_command_handler,
    NULL
};

orbitcalc_command *commands[] = { 
    &help_command,
    &eci_to_ecef_command,
    &ecef_to_eci_command,
    &azimuth_command
};

static int help_command_handler(int argc, char *argv[], void *unused) {
    if(argc != 2) {
        fprintf(stderr, "Error: need command\n");
        return EX_OK;
    }
    for(size_t l=0; l<sizeof commands/sizeof commands[0]; l++) {
        if(!strcmp(commands[l]->name, argv[1])) {
            printf("%s", commands[l]->help);
            return 0;
        }
    }
    fprintf(stderr, "Error: unknown command\n");
    return EX_USAGE;
}

static int longest_command() {
    size_t max = 0;
    for(size_t l=0; l<sizeof commands/sizeof commands[l]; l++) 
        if(strlen(commands[l]->name) > max)
            max = strlen(commands[l]->name);
    return max;
}

static void usage(const char *executable) {
    printf("Usage: %s [OPTION...] <COMMAND> [COMMAND-OPTION...] [ARG...]\n", executable);
    printf("\n");
    printf("Options are:\n");
    printf("-h, --help    : Print this help and exit\n");
    printf("-V, --version : Print the version and exit\n");
    printf("-v, --verbose : Print debug logs\n");
    printf("\n");
    printf("Commands are:\n");
    int sz = longest_command();
    for(size_t l=0; l<sizeof commands/sizeof commands[0]; l++) 
        printf("%-*s : %s\n", sz, commands[l]->name, commands[l]->description);
    printf("\n");
    printf("Use \"%s help <COMMAND>\" to get help on a specific command\n", executable);
    
}

int main(int argc, char *argv[]) {
    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'V' },
        { "verbose", no_argument, NULL, 'v' },
        { NULL }
    };

    opterr = 0;
    int c;
    while((c = getopt_long(argc, argv, "+hvV", longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage(argv[0]);
                exit(EX_OK);
            case 'V':
                printf("%s\n", VERSION);
                exit(EX_OK);
            case 'v':
                debug_enable(1);
                break;
            default:
                fprintf(stderr, "Error: unknown option. \"%s --help\" for help\n", argv[0]);
                exit(EX_USAGE);
        }
    }

    if(argc - optind < 1) {
        fprintf(stderr, "Error: need command. \"%s --help\" for help\n", argv[0]);
        exit(EX_USAGE);
    }

    for(size_t l=0; l<sizeof commands/sizeof commands[0]; l++) {
        if(!strcmp(commands[l]->name, argv[optind])) {
            argc -= optind;
            argv += optind;
#           ifdef __linux__
            optind = 0;
#           else
            optind = 0;
            optreset = 1;
#           endif
            exit(commands[l]->command(argc, argv, commands[l]->extra));
        }
    }

    fprintf(stderr, "Error: unknown command. \"%s --help\" for help\n", argv[0]);
}
