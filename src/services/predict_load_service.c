
#include <stdio.h>
#include <stdlib.h>
#include "predict_load_service.h"
#include "../models/models.h"
#include "../utils/main_utils.h"
#include "../onnx/grid_predictor.h"  // Header generado por onnx2c

PredictLoadService* predict_load_service_new(void) {
    PredictLoadService *service = malloc(sizeof(PredictLoadService));
    if (!service) {
        fprintf(stderr, "[!] Error allocating memory for PredictLoadService\n");
        return NULL;
    }
    // Inicializar cualquier estructura necesaria si aplica
    return service;
}

void predict_load_service_free(PredictLoadService *service) {
    if (service) {
        free(service);
    }
}

float predict_load(PredictLoadService *service, DataPoint *input_data, size_t size) {
    if (!service || !input_data || size != 19) {
        fprintf(stderr, "[!] Invalid input data in model.\n");
        return -1.0f;
    }

    // Preparar el tensor de entrada con dimensiones [1][19][3]
    float input_tensor[1][19][3] = {0};

    for (size_t i = 0; i < size; i++) {
        double day_sin = 0.0f, minute_sin = 0.0f;
        generate_sin_components(input_data[i].timestamp, &day_sin, &minute_sin);
        input_tensor[0][i][0] = (float) day_sin;
        input_tensor[0][i][1] = (float) minute_sin;
        input_tensor[0][i][2] = (float) input_data[i].quantity;
    }

    // Preparar el tensor de salida
    float output_tensor[1][1] = {0};

    // Invocar la función 'entry' del modelo generado por onnx2c
    entry(input_tensor, output_tensor);

    // Retornar el resultado de la predicción
    return output_tensor[0][0];
}