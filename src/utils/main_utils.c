//
// Created by root on 10/30/24.
//

#include "main_utils.h"

#include <math.h>

void generate_sin_components(time_t timestamp, float *day_sin, float *minute_sin) {
    struct tm *tm = gmtime(&timestamp);
    *day_sin = sin((2 * M_PI * tm->tm_yday) / 365.0);
    *minute_sin = sin((2 * M_PI * (tm->tm_hour * 60 + tm->tm_min)) / 1440.0);
}

float normalize(float value, float min, float max) {
    return (value - min) / (max - min);
}

float denormalize(float value, float min, float max) {
    return value * (max - min) + min;
}