#ifndef _UTIL_H_
#define _UTIL_H_

int string_starts_with(char *s, char *prefix);

double deg_to_rad(double a_deg);

double rad_to_deg(double a_rad);

double vec3_len(double vec[3]);

void vec3_scalar_mult(double original[3], double scalar, double result[3]);

void vec3_norm(double original[3], double result[3]);

double dot_product(double a[3], double b[3]);

void cross_product(double a[3], double b[3], double result[3]);

void lat_lon_to_vec3(double lat_deg, double lon_deg, double vec3[3]);

void vec3_copy(double dst[3], const double src[3]);

void vec3_add_to(double dst[3], const double addend[3]);

#endif
