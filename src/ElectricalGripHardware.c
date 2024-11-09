#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "services/main_service.h"

int main(int argc, char *argv[]) {
    if (argc > 1) {        
        const char *command = argv[1];
        if (strcmp(command, "predict_power_grid_load") == 0) {
            const char *auth_token = getenv("POWER_GRID_API_TOKEN");
            if (!auth_token) auth_token = "5503bf77-2158-4b98-85fe-c82a06eee92b";

            MainService *main_service = main_service_new(auth_token);
            printf("[*] Getting power data for the last 24 hours...\n");

            double result = predict_power_grid_load(main_service);
            if (result != -1) {
                printf("[*] Predicted power grid load: %.2f MW\n", result);
            } else {
                fprintf(stderr, "[!] Error predicting power grid load\n");
                return 1;
            }

            main_service_free(main_service);
        } else {
            fprintf(stderr, "[!] Unknown command: %s\n", command);
            return 1;
        }
    } else {
        fprintf(stderr, "[!] No command provided. Please specify a command.\n");
        return 1;
    }

    return 0;
}