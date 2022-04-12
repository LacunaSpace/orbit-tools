int string_starts_with(char *s, char *prefix) {
    while(*s && *prefix) {
        if(*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return !(*prefix);
}
