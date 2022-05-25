#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <sysexits.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "opt_util.h"

#define DEFAULT_CAT_NUMBER (0)
#define DEFAULT_CLASSIFICATION ('U')
#define DEFAULT_LAUNCH_YEAR (2022)
#define DEFAULT_LAUNCH_NUMBER (0)
#define DEFAULT_LAUNCH_PIECE ("A")
#define DEFAULT_BALLISTIC_COEFFICIENT (0.0)
#define DEFAULT_2ND_DERIV_MEAN_MOTION (0.0)
#define DEFAULT_BSTAR (0.0)
#define DEFAULT_ELEMENT_SET_NUMBER (0)

#define DEFAULT_INCLINATION (97.7)
#define DEFAULT_RAAN (0.0)
#define DEFAULT_ECCENTRICITY (0.0)
#define DEFAULT_ARG_OF_PERIGEE (0.0)
#define DEFAULT_MEAN_ANOMALY (0.0)
#define DEFAULT_MEAN_MOTION (15.0)
#define DEFAULT_REVOLUTION_NUMBER (0)


static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...]\n", executable);
    printf("Options are:\n");
    printf("\n");
    printf("General options:\n");
    printf("-h,--help                        : Print this help and exit.\n");
    printf("\n");
    printf("Options to set TLE elements directly:\n");
    printf("   --cat-number=CAT-NUMBER       : Set the catalognumber. The catalognumber\n");
    printf("                                   is a positive integer < 100000. The default\n");
    printf("                                   is %d.\n", DEFAULT_CAT_NUMBER);
    printf("   --classification=CLS          : Set the classification to 'U' (unclassified),\n");
    printf("                                   'C' (classified) or 'S' (secret). The default\n");
    printf("                                   is '%c'\n", DEFAULT_CLASSIFICATION);
    printf("   --launch-year=YEAR            : Set the launch-year, which must be < 10000.\n");
    printf("                                   The default is %d.\n", DEFAULT_LAUNCH_YEAR);
    printf("   --launch-number=NUMBER        : Set the launch-number, which must be less\n");
    printf("                                   than 1000. The default is %d.\n", DEFAULT_LAUNCH_NUMBER);
    printf("   --launch-piece=PIECE          : Set the launch-piece to an alphanumeric\n");
    printf("                                   string of at most 3 characters. The default\n");
    printf("                                   is '%s'.\n", DEFAULT_LAUNCH_PIECE);
    printf("-e,--epoch=EPOCH                 : Set the TLE epoch to the given value, which\n");
    printf("                                   must be specified as YYYY-MM-DDTHH:MM:SSZ or\n");
    printf("                                   alternatively just YYYY-MM-DD. The default is\n");
    printf("                                   the current date and time.\n");
    printf("-b,--ballistic-coefficient=VALUE : Set the ballistic coefficient (also called\n");
    printf("                                   the first derivative of mean motion) to a\n");
    printf("                                   value > -1 and < 1. The default is %g.\n", DEFAULT_BALLISTIC_COEFFICIENT);
    printf("   --2nd-deriv-mean-motion=VALUE : Set the second derivative of mean motion to\n");
    printf("                                   a value > -1E9 and < 1E9.\n");
    printf("                                   The default is %g.\n", DEFAULT_2ND_DERIV_MEAN_MOTION);
    printf("-B,--b-star=VALUE                : Set the value of B* (the drag term) to a value\n");
    printf("                                   greater than -1E9 and less than 1E9. The default\n");
    printf("                                   is %g.\n", DEFAULT_BSTAR);
    printf("   --element-set-number=NUMBER   : Set the element set number. The value is a positive\n");
    printf("                                   integer < 10000. The default is %d.\n", DEFAULT_ELEMENT_SET_NUMBER);
    printf("-i,--inclination=INCLINATION     : Set the inclination, in degrees, to a value >= 0.0 \n");
    printf("                                   and < 180.0. The default is %g.\n", DEFAULT_INCLINATION);
    printf("-r,--raan=RAAN                   : Set the RAAN (right ascension of the ascending node) to\n");
    printf("                                   a value in degrees greater >= 0.0 and < 360.0. The\n");
    printf("                                   default is %g.\n", DEFAULT_RAAN);
    printf("-E,--eccentricity=ECCENTRICIY    : Set the eccentricity to a value greater than or\n");
    printf("                                   equal to 0 and less than 1. The default is %g.\n", DEFAULT_ECCENTRICITY);
    printf("-p,--arg-of-perigee=ARG          : Set the argument of perigee to a value in degrees\n");   
    printf("                                   >= 0.0 and < 360.0. The default is %g.\n", DEFAULT_ARG_OF_PERIGEE);
    printf("-m,--mean-anomaly=MEANANOMALY    : Set the mean anomaly to a value in degrees >= 0.0 and\n");
    printf("                                   < 360.0. The default is %g.\n", DEFAULT_MEAN_ANOMALY);
    printf("-M,--mean-motion=MEANMOTION      : Set the mean motion (number of revolutions per day) to\n");
    printf("                                   a value > 0.0 and < 100.0. The default is %g.\n", DEFAULT_MEAN_MOTION);
    printf("   --revolution-number=REVNUMBER : Set the revolution number at the TLE epoch to\n");
    printf("                                   a positive integer < 100000. The default is %d.\n", DEFAULT_REVOLUTION_NUMBER);
    

}


static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s.\n\n%s --help for help\n", msg, executable);
    exit(EX_USAGE);
}

static double epoch_frac(struct tm *epoch) {
    struct tm year;
    memset(&year, 0, sizeof year);
    year.tm_year = epoch->tm_year;
    year.tm_mon = 0;
    year.tm_mday = 1;
    long frac_secs = timegm(epoch) - timegm(&year);
    int secs_in_day = 24 * 60 * 60;
    return (double)(frac_secs/secs_in_day) + (double)(frac_secs % secs_in_day)/(double)secs_in_day;
}

static void get_current_time(time_t *t) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *t = tv.tv_sec;
}


#define OPT_CATNUMBER (256)
#define OPT_CLASSIFICATION (257)
#define OPT_LAUNCH_YEAR (258)
#define OPT_LAUNCH_NUMBER (259)
#define OPT_LAUNCH_PIECE (260)
#define OPT_2ND_DERIV_MEAN_MOTION (261)
#define OPT_ELEMENT_SET_NUMBER (262)
#define OPT_REVOLUTION_NUMBER (263)

int main(int argc, char *argv[]) {
    executable = argv[0];

    int cat_number = DEFAULT_CAT_NUMBER;
    char classification = DEFAULT_CLASSIFICATION;
    int launch_year = DEFAULT_LAUNCH_YEAR;
    int launch_number = DEFAULT_LAUNCH_NUMBER;
    char *launch_piece = strdup(DEFAULT_LAUNCH_PIECE);
    time_t epoch;
    get_current_time(&epoch);
    struct tm epoch_tm;
    double ballistic_coeff = DEFAULT_BALLISTIC_COEFFICIENT;
    double second_deriv_mean_motion = DEFAULT_2ND_DERIV_MEAN_MOTION;
    double bstar = DEFAULT_BSTAR;
    int element_set_number = DEFAULT_ELEMENT_SET_NUMBER;
    double inclination = DEFAULT_INCLINATION;
    double raan = DEFAULT_RAAN;
    double eccentricity = DEFAULT_ECCENTRICITY;
    double arg_of_perigee = DEFAULT_ARG_OF_PERIGEE;
    double mean_anomaly = DEFAULT_MEAN_ANOMALY;
    double mean_motion = DEFAULT_MEAN_MOTION;
    int revolution_number = DEFAULT_REVOLUTION_NUMBER;
    
    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "cat-number", required_argument, NULL, OPT_CATNUMBER },
        { "classification", required_argument, NULL, OPT_CLASSIFICATION },
        { "launch-year", required_argument, NULL, OPT_LAUNCH_YEAR },
        { "launch-number", required_argument, NULL, OPT_LAUNCH_NUMBER },
        { "launch-piece", required_argument, NULL, OPT_LAUNCH_PIECE },
        { "epoch", required_argument, NULL, 'e' },
        { "ballistic-coefficient", required_argument, NULL, 'b' },
        { "2nd-deriv-mean-motion", required_argument, NULL, OPT_2ND_DERIV_MEAN_MOTION },
        { "b-star", required_argument, NULL, 'B' },
        { "element-set-number", required_argument, NULL, OPT_ELEMENT_SET_NUMBER },
        { "inclination", required_argument, NULL, 'i' },
        { "raan", required_argument, NULL, 'r' },
        { "eccentricity", required_argument, NULL, 'e' },
        { "arg-of-perigee", required_argument, NULL, 'p' },
        { "mean-anomaly", required_argument, NULL, 'm' },
        { "mean-motion", required_argument, NULL, 'M' },
        { "revolution-number", required_argument, NULL, OPT_REVOLUTION_NUMBER },
        { NULL }
    };

    char optstring[] = "+he:b:B:i:r:E:p:m:M:";
    int c;
    opterr = 0;
    while((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case OPT_CATNUMBER:
                if(optarg_as_int(&cat_number, 0, 99999)) 
                    usage_error("Invalid cat-number");
                break;
            case OPT_CLASSIFICATION:
                if(strcmp(optarg, "U") && strcmp(optarg, "C") && strcmp(optarg, "S"))
                    usage_error("Invalid classification");
                classification = *optarg;
                break;
            case OPT_LAUNCH_YEAR:
                if(optarg_as_int(&launch_year, 0, 9999))
                    usage_error("Invalid launch-year");
                break;
            case OPT_LAUNCH_NUMBER:
                if(optarg_as_int(&launch_number, 0, 999))
                    usage_error("Invalid launch-number");
                break;
            case OPT_LAUNCH_PIECE:
                if(strlen(optarg) > 3) 
                    usage_error("Invalid launch-piece");
                free(launch_piece);
                launch_piece = strdup(optarg);
                break;
            case 'e':
                if(optarg_as_datetime_extended(&epoch)) 
                    usage_error("Invalid epoch");
                break;
            case 'b':
                if(optarg_as_double_excl_excl(&ballistic_coeff, -1.0, 1.0))
                    usage_error("Invalid ballistic-coefficient");
                break;
            case OPT_2ND_DERIV_MEAN_MOTION:
                if(optarg_as_double_excl_excl(&second_deriv_mean_motion, -1e9, 1e9))
                    usage_error("Invalid 2nd-deriv-mean-motion");
                break;
            case 'B':
                if(optarg_as_double_excl_excl(&bstar, -1e9, 1e9))
                    usage_error("Invalid bstar");
                break;
            case OPT_ELEMENT_SET_NUMBER:
                if(optarg_as_int(&element_set_number, 0, 9999))
                    usage_error("Invalid element-set-number");
                break;
            case 'i':
                if(optarg_as_double_incl_excl(&inclination, 0.0, 180.0))
                    usage_error("Invalid inclination");
                break;
            case 'r':
                if(optarg_as_double_incl_excl(&raan, 0.0, 360.0))
                    usage_error("Invalid raan");
                break;
            case 'E':
                if(optarg_as_double_incl_excl(&eccentricity, 0.0, 1.0))
                    usage_error("Invalid eccentricity");
                break;
            case 'p':
                if(optarg_as_double_incl_excl(&arg_of_perigee, 0.0, 360.0))
                    usage_error("Invalid arg-of-perigee");
                break;
            case 'm':
                if(optarg_as_double_incl_excl(&mean_anomaly, 0.0, 360.0))
                    usage_error("Invalid mean-anomaly");
                break;
            case 'M':
                if(optarg_as_double_excl_excl(&mean_motion, 0.0, 100.0))
                    usage_error("Invalid mean-motion");
                break;
            case OPT_REVOLUTION_NUMBER:
                if(optarg_as_int(&revolution_number, 0, 99999))
                    usage_error("Invalid revolution-number");
                break;
            case '?':
                usage_error("Invalid option");
                break;             
        }
    }

    gmtime_r(&epoch, &epoch_tm);

    char line1[70];
    sprintf(&line1[0], "1 ");
    sprintf(&line1[2], "%5d", cat_number);
    sprintf(&line1[7], "%c ", classification);
    sprintf(&line1[9], "%02d", launch_year % 100);
    sprintf(&line1[11], "%03d", launch_number);
    sprintf(&line1[14], "%3s ", launch_piece);
    sprintf(&line1[18], "%02d", epoch_tm.tm_year % 100);
    sprintf(&line1[20], "%012.8f ", epoch_frac(&epoch_tm));
    sprintf(&line1[33], "%c.%.08u ", ballistic_coeff < 0.0 ? '-' : ' ',
                                    (unsigned)(100000000 * (fabs(ballistic_coeff) - trunc(fabs(ballistic_coeff)))) );
    char buf[13];
    sprintf(buf, "%11.4e", 10 * second_deriv_mean_motion);
    sprintf(&line1[44], "%c%c%c%c%c%c%c%c ", buf[0], buf[1], buf[3], buf[4], buf[5], buf[6], buf[8], buf[10]);
    sprintf(buf, "%11.4e", 10 * bstar);
    sprintf(&line1[53], "%c%c%c%c%c%c%c%c ", buf[0], buf[1], buf[3], buf[4], buf[5], buf[6], buf[8], buf[10]);
    sprintf(&line1[62], "0 ");
    sprintf(&line1[64], "%4d", element_set_number);

    int checksum = 0;
    for(size_t l=0; l<67; l++) 
        if(line1[l] >= '0' && line1[l] <= '9') checksum += (line1[l] - '0');
        else if(line1[l] == '-') checksum++;
    sprintf(&line1[68], "%d", checksum % 10);

    printf("%s\n", line1);

/*
    for(size_t l=0; l<count; l++) {
        printf("SAT%03zu\n", l);
        printf("1 %05dU %02d%03d%-3s %02d%012.8f %010.8f 00000-0\n", cat_number, launchyear % 100, launchnumber, launchpiece, epoch.tm_year % 100, epoch_frac(&epoch), ballistic_coefficient);
    }
*/
}
