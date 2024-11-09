#ifndef UTILS_H
#define UTILS_H

#include <time.h>

void generate_sin_components(time_t timestamp, double *day_sin, double *minute_sin);
double normalize(double value, double min, double max);
double denormalize(double value, double min, double max);

#endif // UTILS_H