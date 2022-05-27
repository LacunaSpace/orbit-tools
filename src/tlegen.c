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

#define EARTH_RADIUS (6371.0)
#define MAX_RADIUS (100000.0)
#define EARTH_MASS (5.972E24)
#define G (6.67E-11)

#define DEFAULT_NAME ("NONAME")
#define DEFAULT_LINES (3)
#define DEFAULT_MIN_ECCENTRICITY (1E-7)

#define DEFAULT_CAT_NUMBER (90000)
#define DEFAULT_CLASSIFICATION ('U')
#define DEFAULT_LAUNCH_YEAR (2022)
#define DEFAULT_LAUNCH_NUMBER (0)
#define DEFAULT_LAUNCH_PIECE ("A")
#define DEFAULT_BALLISTIC_COEFFICIENT (0.0)
#define DEFAULT_2ND_DERIV_MEAN_MOTION (0.0)
#define DEFAULT_BSTAR (0.0)
#define DEFAULT_ELEMENT_SET_NUMBER (999)

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
    printf("-n,--name=NAME                   : Set the satellite name. The default is %s\n", DEFAULT_NAME);
    printf("-l,--lines=2|3                   : Set the format to 2 or 3 lines. When set\n");
    printf("                                   to 3 lines, the first line contains the\n");
    printf("                                   satellite name. The default is %d.\n", DEFAULT_LINES);
    printf("   --minimum-eccentricity=VALUE  : Is a setting or calculation results in an eccentricity\n");
    printf("                                   below this value, it will be clamped to this value. This\n");
    printf("                                   is because some applications (gpredict) do not handle\n");
    printf("                                   an eccentricity of 0 well. The default value is %g - there's\n", DEFAULT_MIN_ECCENTRICITY);
    printf("                                   little reason to change this other than to set it to 0 to\n");
    printf("                                   switch clamping off.\n");
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
    printf("\n");
    printf("Options to set the orbit radius and automatically infer mean motion\n");
    printf("and eccentricity:\n");
    printf("   --semi-major-axis=VALUE       : Set the orbit's semi major axis, in kilometers.\n");
    printf("   --semi-minor-axis=VALUE       : Set the orbit's semi minor axis, in kilometers.\n");
    printf("                                   Both the semi major and the semi minor axis\n");
    printf("                                   must be greater than the earth's radius (%f km)\n", EARTH_RADIUS);
    printf("                                   and less than %f. The semi minor axis must not be\n", MAX_RADIUS);
    printf("                                   greater than the semi major axis. If only one of\n");
    printf("                                   the two is set, the eccentricity is used to set derive\n");
    printf("                                   the other one. If both are set, the eccentricity is\n");
    printf("                                   derived and will override an eccentricity specified\n");
    printf("                                   with --eccentricity.\n");
    printf("                                   Setting the semi major and/or minor axis will also\n");
    printf("                                   cause the mean motion to be overridden.\n");
    printf("   --apogee-altitude=VALUE       : Set the altitude at apogee, in kilometers. This is the same\n");
    printf("                                   as setting the semi major axis to the sum of the given\n");
    printf("                                   altitude and the earth radius.\n");
    printf("   --perigee-altitude=VALUE      : Set the altitude at perigee, in kilometers. This is the same\n");
    printf("                                   as setting the semi minor axis to the sum of the given\n");
    printf("                                   altitude and the earth radius.\n");
    printf("-a,--altitude=VALUE              : Set the altitude of both apogee and perigee, in kilometers.\n");
    
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
    return 1.0 + (double)(frac_secs/secs_in_day) + (double)(frac_secs % secs_in_day)/(double)secs_in_day;
}

static void get_current_time(time_t *t) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *t = tv.tv_sec;
}

static void set_checksum(char *line) {
    int checksum = 0;
    for(size_t l=0; l<68; l++) 
        if(line[l] >= '0' && line[l] <= '9') checksum += (line[l] - '0');
        else if(line[l] == '-') checksum++;
    sprintf(&line[68], "%d", checksum % 10);
}

#define OPT_CATNUMBER (256)
#define OPT_CLASSIFICATION (257)
#define OPT_LAUNCH_YEAR (258)
#define OPT_LAUNCH_NUMBER (259)
#define OPT_LAUNCH_PIECE (260)
#define OPT_2ND_DERIV_MEAN_MOTION (261)
#define OPT_ELEMENT_SET_NUMBER (262)
#define OPT_REVOLUTION_NUMBER (263)
#define OPT_SEMI_MAJOR_AXIS (264)
#define OPT_SEMI_MINOR_AXIS (265)
#define OPT_PERIGEE_ALTITUDE (266)
#define OPT_APOGEE_ALTITUDE (267)
#define OPT_MINIMUM_ECCENTRICITY (268)

int main(int argc, char *argv[]) {
    executable = argv[0];

    int lines = DEFAULT_LINES;
    char *name = strdup(DEFAULT_NAME);
    double min_eccentricity = DEFAULT_MIN_ECCENTRICITY;
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

    double semi_major_axis;
    int semi_major_axis_set = 0;
    double semi_minor_axis;
    int semi_minor_axis_set = 0;
    
    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "name", required_argument, NULL, 'n' },
        { "lines", required_argument, NULL, 'l' },
        { "minimum-eccentricity", required_argument, NULL, OPT_MINIMUM_ECCENTRICITY },
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
        { "eccentricity", required_argument, NULL, 'E' },
        { "arg-of-perigee", required_argument, NULL, 'p' },
        { "mean-anomaly", required_argument, NULL, 'm' },
        { "mean-motion", required_argument, NULL, 'M' },
        { "revolution-number", required_argument, NULL, OPT_REVOLUTION_NUMBER },
        { "semi-major-axis", required_argument, NULL, OPT_SEMI_MAJOR_AXIS },
        { "semi-minor-axis", required_argument, NULL, OPT_SEMI_MINOR_AXIS },
        { "apogee-altitude", required_argument, NULL, OPT_APOGEE_ALTITUDE },
        { "perigee-altitude", required_argument, NULL, OPT_PERIGEE_ALTITUDE },
        { "altitude", required_argument, NULL, 'a' },
        { NULL }
    };

    char optstring[] = "+hn:l:e:b:B:i:r:E:p:m:M:a:";
    int c;
    opterr = 0;
    while((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'n':
                free(name);
                name = strdup(optarg);
                break;
            case 'l':
                if(!strcmp(optarg, "2")) lines = 2;
                else if(!strcmp(optarg, "3")) lines = 3;
                else usage_error("Invalid lines");
                break;
            case OPT_MINIMUM_ECCENTRICITY:
                if(optarg_as_double_incl_excl(&min_eccentricity, 0.0, 1.0))
                    usage_error("Invalid minimum-eccentricity");
                break;
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
            case OPT_SEMI_MAJOR_AXIS:
                if(optarg_as_double_incl_excl(&semi_major_axis, EARTH_RADIUS, MAX_RADIUS))
                    usage_error("Invalid semi-major-axis");
                semi_major_axis_set = 1;
                break;
            case OPT_SEMI_MINOR_AXIS:
                if(optarg_as_double_incl_excl(&semi_minor_axis, EARTH_RADIUS, MAX_RADIUS))
                    usage_error("Invalid semi-minor-axis");
                semi_minor_axis_set = 1;
                break;
            case OPT_APOGEE_ALTITUDE: {
                double v;
                if(optarg_as_double_incl_excl(&v, 0.0, MAX_RADIUS-EARTH_RADIUS))
                    usage_error("Invalid apogee-altitude");
                semi_major_axis = v + EARTH_RADIUS;
                semi_major_axis_set = 1;
                break;
            }
            case OPT_PERIGEE_ALTITUDE: {
                double v;
                if(optarg_as_double_incl_excl(&v, 0.0, MAX_RADIUS-EARTH_RADIUS))
                    usage_error("Invalid perigee-altitude");
                semi_minor_axis = v + EARTH_RADIUS;
                semi_minor_axis_set = 1;
                break;
            }
            case 'a': {
                double v;
                if(optarg_as_double_incl_excl(&v, 0.0, MAX_RADIUS-EARTH_RADIUS))
                    usage_error("Invalid altitude");
                semi_minor_axis = v + EARTH_RADIUS;
                semi_major_axis = v + EARTH_RADIUS;
                semi_minor_axis_set = 1;
                semi_major_axis_set = 1;
                break;
            }
            case '?':
                usage_error("Invalid option");
                break;             
        }
    }

    gmtime_r(&epoch, &epoch_tm);

    if(semi_major_axis_set && semi_minor_axis_set) {
        if(semi_minor_axis > semi_major_axis) usage_error("Invalid semi-minor-axis");
        double b_over_a = semi_minor_axis / semi_major_axis;
        eccentricity = (b_over_a - 1) / (b_over_a + 1);
    } else if(semi_major_axis_set) {
        semi_minor_axis = semi_major_axis * (1.0 - eccentricity) / (1.0 + eccentricity);
    } else if(semi_minor_axis_set) {
        semi_major_axis = semi_minor_axis * (1.0 + eccentricity) / (1.0 - eccentricity);
    }

    if(semi_major_axis_set || semi_minor_axis_set) {
        double mean_motion_rad_sec = sqrt((G * EARTH_MASS) / pow(semi_major_axis * 1000.0, 3.0));
        mean_motion = 24 * 60 * 60 * mean_motion_rad_sec / (2.0 * M_PI);
    }

    if(eccentricity < min_eccentricity)
        eccentricity = min_eccentricity;

    if(lines == 3) printf("%s\n", name);

    char line1[70];
    sprintf(&line1[0], "1 ");
    sprintf(&line1[2], "%5d", cat_number);
    sprintf(&line1[7], "%c ", classification);
    sprintf(&line1[9], "%02d", launch_year % 100);
    sprintf(&line1[11], "%03d", launch_number);
    sprintf(&line1[14], "%-3s ", launch_piece);
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

    set_checksum(line1);

    char line2[70];
    sprintf(&line2[0], "2 ");
    sprintf(&line2[2], "%5d ", cat_number);
    sprintf(&line2[8], "%8.4f ", inclination);
    sprintf(&line2[17], "%8.4f ", raan);
    sprintf(buf, "%9.7f", eccentricity);
    sprintf(&line2[26], "%s ", &buf[2]);
    sprintf(&line2[34], "%8.4f ", arg_of_perigee);
    sprintf(&line2[43], "%8.4f ", mean_anomaly);
    sprintf(&line2[52], "%11.8f ", mean_motion);
    sprintf(&line2[63], "%5d", revolution_number);

    set_checksum(line2);

    printf("%s\n%s\n", line1, line2);

/*
    for(size_t l=0; l<count; l++) {
        printf("SAT%03zu\n", l);
        printf("1 %05dU %02d%03d%-3s %02d%012.8f %010.8f 00000-0\n", cat_number, launchyear % 100, launchnumber, launchpiece, epoch.tm_year % 100, epoch_frac(&epoch), ballistic_coefficient);
    }
*/
}
