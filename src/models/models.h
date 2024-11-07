//
// Created by root on 10/31/24.
//

#ifndef ELECTRICALGRIPHARDWARE_MODELS_H
#define ELECTRICALGRIPHARDWARE_MODELS_H

#include <time.h>

typedef struct {
    time_t timestamp;
    float quantity;
} Point;

typedef struct {
    Point *points;
    size_t size;
} Period;

typedef struct {
    Period period;
} TimeSeries;

typedef struct {
    TimeSeries time_series;
} GLMarketDocument;

#endif //ELECTRICALGRIPHARDWARE_MODELS_H
