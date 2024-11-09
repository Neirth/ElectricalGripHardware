#ifndef PREDICT_LOAD_SERVICE_H
#define PREDICT_LOAD_SERVICE_H

#include <stddef.h>
#include "../utils/main_utils.h"
#include "../models/models.h"

typedef struct {
} PredictLoadService;

PredictLoadService* predict_load_service_new(void);
void predict_load_service_free(PredictLoadService *service);
float predict_load(PredictLoadService *service, DataPoint *input_data, size_t size);

#endif // PREDICT_LOAD_SERVICE_H