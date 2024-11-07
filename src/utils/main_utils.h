#ifndef UTILS_H
#define UTILS_H

#include <time.h>

void generate_sin_components(time_t timestamp, float *day_sin, float *minute_sin);
float normalize(float value, float min, float max);
float denormalize(float value, float min, float max);

#endif // UTILS_H