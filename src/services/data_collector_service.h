#ifndef DATA_COLLECTOR_SERVICE_H
#define DATA_COLLECTOR_SERVICE_H

#include <time.h>
#include <curl/curl.h>
#include "../models/models.h"

typedef struct {
    CURL *curl;
    char *auth_token;
} DataCollectorService;

DataCollectorService* data_collector_service_new(const char *auth_token);
void data_collector_service_free(DataCollectorService *service);
int get_power_data(DataCollectorService *service, time_t start, time_t end, Point **points, size_t *size);

#endif // DATA_COLLECTOR_SERVICE_H