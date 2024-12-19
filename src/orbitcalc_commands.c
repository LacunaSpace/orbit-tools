#include "orbitcalc.h"
#include "orbitcalc_commands.h"
#include "debug.h"
#include "opt_util.h"
#include "util.h"
#include <stdio.h>
#include <sysexits.h>
#include <getopt.h>
#include <sys/time.h>

#define ECI_TO_ECEF (0)
#define ECEF_TO_ECI (1)

static int coord_convert_command_handler(int argc, char *argv[], void *arg) {
    struct option longopts[] = {
        { "time", required_argument, NULL, 't' },
        { NULL }
    };
    DEBUG("eci_to_ecef");
    for(size_t l=0; l<argc; l++) DEBUG("%zu: %s", l, argv[l]);
    int c;

    /* Defaut to current time - we only use the tv_sec portion of the struct */
    struct timeval t;
    gettimeofday(&t, NULL);
    
    while((c = getopt_long(argc, argv, "+t:", longopts, NULL)) != -1) {
        switch(c) {
            case 't':
                if(optarg_as_datetime_extended(&t.tv_sec)) {
                    fprintf(stderr, "Error: invalid --time\n");
                    return EX_USAGE;
                }
                break;
            default:
                fprintf(stderr, "Error: unknown option\n");
                return EX_USAGE;
        }
    }
    DEBUG("time=%d", t.tv_sec);

    if(optind != argc-1) {
        fprintf(stderr, "Error: expected one argument (X,Y,Z without whitespace)\n");
        return EX_USAGE;
    }

    double src[3], dst[3];
    if(arg_as_vec3(argv[optind], src)) {
        fprintf(stderr, "Error: invalid argument\n");
        return EX_USAGE;
    }
    if(arg == ECI_TO_ECEF) 
        eci_to_ecef(src, (double)t.tv_sec, dst);
    else
        ecef_to_eci(src, (double)t.tv_sec, dst);
    printf("%g,%g,%g\n", dst[0], dst[1], dst[2]);
    return EX_OK;
}

orbitcalc_command eci_to_ecef_command = {
    "eci-to-ecef",
    "Converts a 3-d vector from the ECI into the ECEF coordinate system",
    "Usage: eci-to-ecef [OPTION...] <VECTOR>\n"
    "\n"
    "Options are:\n"
    "-t, --time=<TIME> : Sets the date and time at which to perform the conversion,\n"
    "                    formatted as yyyy-mm-ddThh:mm:ssZ. The default value is the\n"
    "                    current date and time\n"
    "\n"
    "<VECTOR> is specified as an X, Y and Z coordinate seperated by commas, without\n"
    "         whitespace. They are in kilometers. If the X coordinate is negative (and\n"
    "         therefore starts with a '-'), separate options and arguments by '--'\n",
    coord_convert_command_handler,
    (void *)ECI_TO_ECEF
};


orbitcalc_command ecef_to_eci_command = {
    "ecef-to-eci",
    "Converts a 3-d vector from the ECEF into the ECI coordinate system",
    "Usage: ecef-to-eci [OPTION...] <VECTOR>\n"
    "\n"
    "Options are:\n"
    "-t, --time=<TIME> : Sets the date and time at which to perform the conversion,\n"
    "                    formatted as yyyy-mm-ddThh:mm:ssZ. The default value is the\n"
    "                    current date and time\n"
    "\n"
    "<VECTOR> is specified as an X, Y and Z coordinate seperated by commas, without\n"
    "         whitespace. They are in kilometers. If the X coordinate is negative (and\n"
    "         therefore starts with a '-'), separate options and arguments by '--'\n",
    coord_convert_command_handler,
    (void *)ECEF_TO_ECI
};


