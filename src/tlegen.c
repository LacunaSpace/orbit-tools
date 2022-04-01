#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <sysexits.h>
#include <time.h>
#include <string.h>

#define DEFAULT_RADIUS (500)
#define MAX_RADIUS (100000)

#define DEFAULT_CATNUMBER (0)

#define DEFAULT_LAUNCHYEAR (2022)
#define DEFAULT_LAUNCHNUMBER (0)
#define DEFAULT_LAUNCHPIECE ("A")

#define DEFAULT_EPOCH ("2022-01-01T00:00:00Z")

#define DEFAULT_BALLISTIC_COEFFICIENT (0.0)

#define DEFAULT_INCLINATION (0.0)


#define MAX_COUNT (1000)

static char *executable;

static void usage(void) {
    printf("Usage: %s [OPTION...]\n", executable);
    printf("Options are:\n");
    printf("\t-h,--help:\tprint this help and exit\n");
    printf("\t-c,--count=COUNT:\tgenerate COUNT TLEs. The default value is 1.\n");

    printf("\t-e,--epoch=DATETIME:\tsets the TLE epoch.\n");

    printf("\t-b,--ballistic-coefficient=BALLISTICCOEFFICIENT:\tsets the ballistic coefficient as a\n");
    printf("\t\t\tfloat >= 0 and < 10. Default value is %f.\n", DEFAULT_BALLISTIC_COEFFICIENT); 

    printf("\t-r,--radius=RADIUS:\tsets the orbit's radius, in kilometers. The default value is %d.\n", DEFAULT_RADIUS);
    printf("\t-R,--radius-increment=RADIUSINCREMENT:\tsets the radius increment, that is the number of\n");
    printf("\t\t\t\tkilometers to add to the radius for each subsequent TLE when generating multiple TLEs.\n");
    printf("\t\t\t\tThe default value is 0.\n");
}

static int optarg_as_int(int *value, int min, int max) {
    char *s;
    int v = strtol(optarg, &s, 10);
    if(*s || v < min || v > max) return -1;
    *value = v;
    return 0;
}

static int optarg_as_double(double *value, double min_inc, double max_excl) {
    char *s;
    double v = strtod(optarg, &s);
    if(*s || v < min_inc || v >= max_excl) return -1;
    *value = v;
    return 0;
}


static int string_as_datetime(char *s, struct tm *timeptr) {
    memset(timeptr, 0, sizeof(struct tm));
    char *p = strptime(s, "%Y-%m-%dT%H:%M:%SZ", timeptr);
    if(p && !*p) return 0;

    return -1;
}

static int optarg_as_datetime(struct tm *timeptr) {
    return string_as_datetime(optarg, timeptr);
}

static void usage_error(char *msg) {
    fprintf(stderr, "Error: %s.\n%s --help for help\n", msg, executable);
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

int main(int argc, char *argv[]) {
    executable = argv[0];

    struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "count", required_argument, NULL, 'c' },
        { "epoch", required_argument, NULL, 'e' },
        { "ballistic-coefficient", required_argument, NULL, 'b' },
        { NULL }
    };

    char optstring[] = ":hc:e:b:";

    int count = 1;
    int cat_number = DEFAULT_CATNUMBER;
    double inclination = DEFAULT_INCLINATION;
    int launchyear = DEFAULT_LAUNCHYEAR;
    int launchnumber = DEFAULT_LAUNCHNUMBER;
    char *launchpiece = DEFAULT_LAUNCHPIECE;
    struct tm epoch;
    string_as_datetime(DEFAULT_EPOCH, &epoch);
    double ballistic_coefficient = DEFAULT_BALLISTIC_COEFFICIENT;

    int c;
    opterr = 0;
    while((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch(c) {
            case 'h':
                usage();
                exit(0);
            case 'c':
                if(optarg_as_int(&count, 1, MAX_COUNT))
                    usage_error("Invalid count");
                break;
            case 'e':
                if(optarg_as_datetime(&epoch))
                    usage_error("Invalid epoch");
                break;
            case 'b':
                if(optarg_as_double(&ballistic_coefficient, 0.0, 10.0))
                    usage_error("Invalid ballistic coefficient");
                break;
        }
    }

    for(size_t l=0; l<count; l++) {
        printf("SAT%03zu\n", l);
        printf("1 %05dU %02d%03d%-3s %02d%012.8f %010.8f 00000-0\n", cat_number, launchyear % 100, launchnumber, launchpiece, epoch.tm_year % 100, epoch_frac(&epoch), ballistic_coefficient);
    }
}
