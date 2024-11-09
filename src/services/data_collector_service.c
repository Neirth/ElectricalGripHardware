#include <curl/curl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types/vxTypes.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "../models/models.h"

#define MAX_POINTS 19

typedef struct {
    CURL *curl;
    char *auth_token;
} DataCollectorService;

// Inicialización del servicio
DataCollectorService* data_collector_service_new(const char *auth_token) {
    DataCollectorService *service = malloc(sizeof(DataCollectorService));
    service->curl = curl_easy_init();
    service->auth_token = strdup(auth_token);
    return service;
}

// Liberación del servicio
void data_collector_service_free(DataCollectorService *service) {
    if (service->curl) curl_easy_cleanup(service->curl);
    if (service->auth_token) free(service->auth_token);
    free(service);
}

// Creación del cuerpo XML para la solicitud
char* create_request_body(time_t start, time_t end) {
    char start_str[30], end_str[30];
    strftime(start_str, sizeof(start_str), "%Y-%m-%dT%H:%M:%SZ", gmtime(&start));
    strftime(end_str, sizeof(end_str), "%Y-%m-%dT%H:%M:%SZ", gmtime(&end));

    char *body = malloc(2048);
    snprintf(body, 2048,
        "<StatusRequest_MarketDocument xmlns=\"urn:iec62325.351:tc57wg16:451-5:statusrequestdocument:4:0\">"
        "<mRID>SampleCallToRestfulApi</mRID>"
        "<type>A59</type>"
        "<sender_MarketParticipant.mRID codingScheme=\"A01\">10X1001A1001A450</sender_MarketParticipant.mRID>"
        "<sender_MarketParticipant.marketRole.type>A07</sender_MarketParticipant.marketRole.type>"
        "<receiver_MarketParticipant.mRID codingScheme=\"A01\">10X1001A1001A450</receiver_MarketParticipant.mRID>"
        "<receiver_MarketParticipant.marketRole.type>A32</receiver_MarketParticipant.marketRole.type>"
        "<createdDateTime>2016-01-10T13:00:00Z</createdDateTime>"
        "<AttributeInstanceComponent>"
            "<attribute>DocumentType</attribute>"
            "<attributeValue>A65</attributeValue>"
        "</AttributeInstanceComponent>"
        "<AttributeInstanceComponent>"
            "<attribute>ProcessType</attribute>"
            "<attributeValue>A16</attributeValue>"
        "</AttributeInstanceComponent>"
        "<AttributeInstanceComponent>"
            "<attribute>OutBiddingZone_Domain</attribute>"
            "<attributeValue>10YAT-APG------L</attributeValue>"
        "</AttributeInstanceComponent>"
        "<AttributeInstanceComponent>"
            "<attribute>TimeInterval</attribute>"
            "<attributeValue>%s/%s</attributeValue>"
        "</AttributeInstanceComponent>"
        "</StatusRequest_MarketDocument>",
        start_str, end_str
    );
    return body;
}

struct response {
    char *memory;
    size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    struct response *mem = (struct response *)userp;
    size_t realsize = size * nmemb;

    // Solo inicializar a NULL la primera vez
    if (mem->memory == NULL) {
        mem->size = 0;
    }

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("[*] not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Asumiendo que estas funciones están definidas en xmlLib.h
xmlDocPtr parse_xml_response(const char* xml_string) {
    if (!xml_string) return NULL;

    printf("[*] Parsing XML response\n");
    
    xmlDocPtr doc = xmlParseMemory(xml_string, strlen(xml_string));
    if (!doc) {
        fprintf(stderr, "Error al parsear XML\n");
        return NULL;
    }
    return doc;
}

long calculate_timestamp_from_position(time_t start_time, int position) {
    // Cada posición es 15 minutos antes que la siguiente
    return start_time - (15 * 60 * (position - 1));
}

time_t round_up_to_nearest_15_minutes(time_t time) {
    struct tm *lt = gmtime(&time);
    int minutes = lt->tm_min;
    int extra_minutes = (15 - (minutes % 15)) % 15;

    time_t rounded_time = time + extra_minutes * 60;
    return rounded_time;
}

STATUS extract_data_points(xmlDocPtr doc, DataPoint* data_points, size_t *size) {
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root) {
        return -1;
    }

    xmlNodePtr timeSeriesNode = NULL;
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"TimeSeries") == 0) {
            timeSeriesNode = node;
            break;
        }
    }
    if (!timeSeriesNode) {
        return -1;
    }

    xmlNodePtr periodNode = NULL;
    for (xmlNodePtr node = timeSeriesNode->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"Period") == 0) {
            periodNode = node;
            break;
        }
    }
    if (!periodNode) {
        return -1;
    }

    // Recorre hasta el final del nodo Period para localizar el último nodo Point
    xmlNodePtr pointNode = NULL;
    int totalPoints = 0;

    for (xmlNodePtr node = periodNode->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"Point") == 0) {
            pointNode = node;
            totalPoints++;
        }
    }

    if (totalPoints == 0) {
        return -1;
    }

    int count = 0;
    DataPoint temp_points[MAX_POINTS]; // Limitar a 19 puntos
    memset(temp_points, 0, sizeof(temp_points));

    // Obtener el tiempo actual redondeado al siguiente intervalo de 15 minutos
    time_t now = time(NULL);
    time_t start_time = round_up_to_nearest_15_minutes(now);

    // Recorrer desde el último nodo Point hasta un máximo de 19 nodos hacia atrás
    for (xmlNodePtr node = pointNode; node && count < 19; node = node->prev) {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"Point") == 0) {
            xmlNodePtr positionNode = NULL;
            xmlNodePtr quantityNode = NULL;

            for (xmlNodePtr child = node->children; child; child = child->next) {
                if (child->type == XML_ELEMENT_NODE) {
                    if (xmlStrcmp(child->name, (const xmlChar *)"position") == 0) {
                        positionNode = child;
                    } else if (xmlStrcmp(child->name, (const xmlChar *)"quantity") == 0) {
                        quantityNode = child;
                    }
                }
            }

            if (positionNode && quantityNode) {
                xmlChar *position_str = xmlNodeGetContent(positionNode);
                xmlChar *quantity_str = xmlNodeGetContent(quantityNode);

                int position = atoi((char *)position_str);
                double quantity = atof((char *)quantity_str);

                temp_points[count].quantity = quantity;
                temp_points[count].timestamp = calculate_timestamp_from_position(start_time, position);
                count++;

                xmlFree(position_str);
                xmlFree(quantity_str);
            }
        }
    }

    // Invertir el arreglo para tener los datos en orden cronológico
    for (int i = 0; i < count; i++) {
        data_points[i] = temp_points[count - i - 1];
    }

    *size = count;
    return 0;
}

int parse_response(const char* response_text, DataPoint* data_points, size_t *size, long start) {
    xmlDocPtr doc = parse_xml_response(response_text);
    if (!doc) {
        return -1;
    }
    
    int result = extract_data_points(doc, data_points, size);

    if (result != 0) {
        fprintf(stderr, "[!] Error extracting data points\n");
    }
    xmlFreeDoc(doc);
    
    return result;
}

// Función principal para obtener datos de potencia
int get_power_data(DataCollectorService *service, time_t start, time_t end, DataPoint *data_points, size_t *size) {
    CURLcode res;
    char error_buffer[CURL_ERROR_SIZE];
    char *url = "https://web-api.tp.entsoe.eu/api";
    char *body = create_request_body(start, end);
    struct response chunk = {NULL, 0};

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/xml");
    char token_header[256];
    snprintf(token_header, sizeof(token_header), "SECURITY_TOKEN: %s", service->auth_token);
    headers = curl_slist_append(headers, token_header);

    struct curl_slist *host = NULL;
    host = curl_slist_append(NULL, "web-api.tp.entsoe.eu:80:90.183.125.159");
    host = curl_slist_append(NULL, "web-api.tp.entsoe.eu:443:90.183.125.159");

    curl_easy_setopt(service->curl, CURLOPT_URL, url);
    curl_easy_setopt(service->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(service->curl, CURLOPT_RESOLVE, host);
    curl_easy_setopt(service->curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(service->curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(service->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(service->curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(service->curl, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(service->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(service->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    res = curl_easy_perform(service->curl);

    if (body) {
        free(body);
    }

    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        fprintf(stderr, "[!] Error in curl_easy_perform: %s\n", error_buffer);
        if (chunk.memory) {
            free(chunk.memory);
        }
        return -1;
    }

    if (!chunk.memory) {
        fprintf(stderr, "[!] No data received\n");
        return -1;
    }

    int parse_result = parse_response(chunk.memory, data_points, size, start);
    if (parse_result != 0) {
        fprintf(stderr, "[!] Error parsing response\n");
        free(chunk.memory);
        return -1;
    }

    free(chunk.memory);
    return 0;
}