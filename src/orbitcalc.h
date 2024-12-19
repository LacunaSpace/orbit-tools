#ifndef ORBITCALC_H
#define ORBITCALC_H

typedef struct {
    const char *name;
    const char *description;
    const char *help;
    int(*command)(int argc, char *argv[], void *extra);
    void *extra;
} orbitcalc_command;

#endif
