#include <curl/curl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>

typedef struct {
    CURL *curl;
    char *auth_token;
} DataCollectorService;

typedef struct {
    long timestamp;
    float quantity;
} DataPoint;

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

// Callback de cURL para almacenar respuesta
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **response_ptr =  (char**)userp;
    strncat(*response_ptr, contents, realsize);
    return realsize;
}

// Asumiendo que estas funciones están definidas en xmlLib.h
xmlDocPtr parse_xml_response(const char* xml_string) {
    if (!xml_string) return NULL;
    
    xmlDocPtr doc = xmlParseMemory(xml_string, strlen(xml_string));
    if (!doc) {
        fprintf(stderr, "Error al parsear XML\n");
        return NULL;
    }
    return doc;
}

int extract_data_points(xmlDocPtr doc, DataPoint* data_points, int* num_points) {
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root) {
        fprintf(stderr, "Documento XML vacío\n");
        return -1;
    }

    int count = 0;
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && strcmp((char*)node->name, "data") == 0) {
            xmlChar* timestamp_str = xmlGetProp(node, (xmlChar*)"timestamp");
            xmlChar* quantity_str = xmlGetProp(node, (xmlChar*)"quantity");
            
            if (timestamp_str && quantity_str) {
                data_points[count].timestamp = atol((char*)timestamp_str);
                data_points[count].quantity = atof((char*)quantity_str);
                count++;
            }
            
            xmlFree(timestamp_str);
            xmlFree(quantity_str);
        }
    }
    
    *num_points = count;
    return 0;
}

int parse_response(const char* response_text, DataPoint* data_points, int* num_points, long start) {
    xmlDocPtr doc = parse_xml_response(response_text);
    if (!doc) {
        return -1;
    }
    
    int result = extract_data_points(doc, data_points, num_points);
    xmlFreeDoc(doc);
    
    return result;
}

// Función principal para obtener datos de potencia
int get_power_data(DataCollectorService *service, time_t start, time_t end, DataPoint **data_points, int *num_points) {
    CURLcode res;
    char error_buffer[CURL_ERROR_SIZE];
    char *url = "https://web-api.tp.entsoe.eu/api";
    char *body = create_request_body(start, end);
    char *response_text = NULL;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/xml");
    char token_header[256];
    snprintf(token_header, sizeof(token_header), "SECURITY_TOKEN: %s", service->auth_token);
    headers = curl_slist_append(headers, token_header);

    curl_easy_setopt(service->curl, CURLOPT_URL, url);
    curl_easy_setopt(service->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(service->curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(service->curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(service->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(service->curl, CURLOPT_WRITEDATA, &response_text);
    curl_easy_setopt(service->curl, CURLOPT_ERRORBUFFER, error_buffer);

    res = curl_easy_perform(service->curl);

    if (body) {
        free(body);
    }

    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        fprintf(stderr, "Error en cURL: %s\n", error_buffer);
        if (response_text) {
            free(response_text);
        }
        return -1;
    }

    if (!response_text) {
        fprintf(stderr, "No se recibió respuesta del servidor\n");
        return -1;
    }

    int parse_result = parse_response(response_text, *data_points, num_points, start);
    if (parse_result != 0) {
        fprintf(stderr, "Error al parsear la respuesta\n");
        free(response_text);
        return -1;
    }

    free(response_text);
    return 0;
}