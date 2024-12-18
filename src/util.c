#include <math.h>
#include <stddef.h>
#include <string.h>

int string_starts_with(char *s, char *prefix) {
    while(*s && *prefix) {
        if(*s != *prefix) return 0;
        s++;
        prefix++;
    }
    return !(*prefix);
}

double deg_to_rad(double a_deg) {
    return a_deg * (double)M_PI/180.0;
}

double rad_to_deg(double a_rad) {
    return a_rad * 180.0 / (double)M_PI;
}

double vec3_len(double vec[3]) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

void vec3_scalar_mult(double original[3], double scalar, double result[3]) {
    for(size_t l=0; l<3; l++)
        result[l] = original[l] * scalar;
}

void vec3_norm(double original[3], double result[3]) {
    double len = vec3_len(original);
    vec3_scalar_mult(original, 1.0/len, result);
}

double dot_product(double a[3], double b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void cross_product(double a[3], double b[3], double result[3]) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2];
    result[2] = a[0]*b[1] - a[1]*b[0];
}

void lat_lon_to_vec3(double lat_deg, double lon_deg, double vec3[3]) {
    double lat = deg_to_rad(lat_deg),
           lon = deg_to_rad(lon_deg);
    vec3[0] = cos(lat) * cos(lon),
    vec3[1] = cos(lat) * sin(lon),
    vec3[2] = sin(lat);
}

void vec3_copy(double dst[3], const double src[3]) {
    memcpy(dst, src, sizeof(double) * 3);
}

void vec3_add_to(double dst[3], const double addend[3]) {
    for(size_t l=0; l<3; l++)
        dst[l] += addend[l];
}
