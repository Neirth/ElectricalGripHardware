#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "./data_collector_service.h"
#include "./predict_load_service.h"
#include "../utils/main_utils.h"
#include "../models/models.h"

#define FLT_MAX 3.40282347e+38F
#define FLT_MIN 1.17549435e-38F
#define MAX_POINTS 19

typedef struct {
    DataCollectorService *data_service;
    PredictLoadService *inference_service;
} MainService;

MainService* main_service_new(const char *auth_token) {
    MainService *service = malloc(sizeof(MainService));
    service->data_service = data_collector_service_new(auth_token);
    service->inference_service = predict_load_service_new();
    return service;
}

void main_service_free(MainService *service) {
    data_collector_service_free(service->data_service);
    predict_load_service_free(service->inference_service);
    free(service);
}

double predict_power_grid_load(MainService *service) {
    time_t end = time(NULL);
    struct tm *end_tm = gmtime(&end);
    end_tm->tm_hour = 23;
    end_tm->tm_min = 45;
    end_tm->tm_sec = 0;
    end = mktime(end_tm);

    struct tm start_tm = *end_tm;
    start_tm.tm_hour = 0;
    start_tm.tm_min = 0;
    start_tm.tm_sec = 0;
    time_t start = mktime(&start_tm);

    DataPoint *points = malloc(sizeof(DataPoint) * MAX_POINTS);

    size_t size;
    if (get_power_data(service->data_service, start, end, points, &size) != 0) {
        fprintf(stderr, "[!] Error fetching power data\n");
        return -1;
    }

    double min = FLT_MAX, max = -FLT_MAX;
    for (size_t i = 0; i < MAX_POINTS; i++) {
        if (points[i].quantity < min) min = points[i].quantity;
        if (points[i].quantity > max) max = points[i].quantity;
    }

    for (size_t i = 0; i < size; i++) {
        points[i].quantity = normalize(points[i].quantity, min, max);
    }

    double result = predict_load(service->inference_service, points, size);
    result = denormalize(result, min, max);

    free(points);
    return result;
}