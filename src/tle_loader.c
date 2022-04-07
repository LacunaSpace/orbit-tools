#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "tle_loader.h"

loaded_tle *load_tles(FILE *in) {
    loaded_tle *lt = NULL;
    loaded_tle **tail = &lt;
    char *name = NULL, *line1 = NULL;
    for(;;) {
        char *line = NULL;
        size_t cap;
        ssize_t result = getline(&line, &cap, in);
        if(result < 0) break;
        for(ssize_t l=result-1; l >= 0 && line[l] <= 0x20; l--) line[l] = 0;
        if(line1 && strlen(line) == 69 && strstr(line, "2 ") == line) {
            /* We have a line1, possibly a name and this looks like a line2 */
            loaded_tle *next = malloc(sizeof(loaded_tle));
            next->name = name;
            parseLines(&next->tle, line1, line);
            free(line1);
            line1 = NULL; name = NULL;
            *tail = next;
            tail = &next->next;
            next->next = NULL;
        } else if(strlen(line) == 69 && strstr(line, "1 ") == line) {
            /* This looks like a line1, we may also have a name */
            line1 = line;
        } else {
            free(name);
            free(line1);
            line1 = NULL; name = NULL;
            if(strlen(line) <= 24) 
                name = line;
        }
    }
    free(name);
    free(line1);

    if(feof(in)) {
        return lt;
    } else {
        unload_tles(lt);
        return NULL;
    }
}

loaded_tle *load_tles_from_filename(char *filename) {
    FILE *in;
    if(!strcmp("-", filename)) in = stdin;
    else in = fopen(filename, "r");
    if(!in) return NULL;

    loaded_tle *lt = load_tles(in);
    if(in != stdin) fclose(in);

    return lt;
}

void unload_tles(loaded_tle *lt) {
    while(lt) {
        loaded_tle *next = lt->next;
        free(lt->name);
        lt = next;
    }
}

loaded_tle *get_tle_by_name(loaded_tle *lt, char *name) {
    if(!name) 
        return lt;
    while(lt) {
        if(lt->name && !strcmp(name, lt->name)) return lt;
        lt = lt->next;
    }
    return NULL;
}

loaded_tle *get_tle_by_index(loaded_tle *lt, size_t index) {
    for(; index; index--) lt=lt->next;
    return lt;
}

int count_tles(loaded_tle *lt) {
    int count = 0;
    while(lt) {
        count++;
        lt = lt->next;
    }
    return count;
}
