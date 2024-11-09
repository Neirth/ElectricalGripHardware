#ifndef MAIN_SERVICE_H
#define MAIN_SERVICE_H

#include "data_collector_service.h"
#include "predict_load_service.h"

typedef struct {
    DataCollectorService *data_service;
    PredictLoadService *inference_service;
} MainService;

MainService* main_service_new(const char *auth_token);
void main_service_free(MainService *service);
double predict_power_grid_load(MainService *service);

#endif // MAIN_SERVICE_H