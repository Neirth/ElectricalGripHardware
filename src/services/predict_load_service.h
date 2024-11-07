#ifndef PREDICT_LOAD_SERVICE_H
#define PREDICT_LOAD_SERVICE_H

#include <stddef.h>
#include "../utils/main_utils.h"  // Asegúrate de incluir la definición de Point
#include "../models/models.h"

typedef struct {
    // Si necesitas agregar miembros a la estructura, hazlo aquí
    // Por ejemplo, un espacio de trabajo si es requerido por el modelo
} PredictLoadService;

PredictLoadService* predict_load_service_new(void);
void predict_load_service_free(PredictLoadService *service);
float predict_load(PredictLoadService *service, Point *input_data, size_t size);

#endif // PREDICT_LOAD_SERVICE_H