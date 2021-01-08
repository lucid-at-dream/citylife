#include "service.h"
#include "json.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_SIZE 10240

char validate_service_request(json_object *request);
service *create_service_from_request(json_object *request);

service *service_new_from_json(char *json)
{
    json_object *request = parse_json(json);

    // Validate request
    char is_valid = validate_service_request(request);

    // Create service object from json request
    service *svc = create_service_from_request(request);

    json_dealloc(request);
    return svc;
}

/**
 * TODO
 */
char validate_service_request(json_object *request) {
    return 1;
}

char *copy_string(char *original) {
    int str_size = strnlen(original, MAX_STRING_SIZE);
    char *buffer = (char *)calloc(str_size + 1, sizeof(char));
    strncpy(buffer, original, (str_size + 1) * sizeof(char));
    return buffer;
}

void parse_endpoint_arg(map *m, char *arg_name, service_endpoint *endpoint) {

    printf("Parsing endpoint arg %s\n", arg_name);

    if (strcmp(arg_name, "__doc__") == 0) {
        return;
    }

    service_endpoint_arg *arg = (service_endpoint_arg *)calloc(1, sizeof(service_endpoint_arg));

    map *arg_desc = ((json_object *)map_get(m, arg_name))->content.object;

    arg->name = copy_string(arg_name);
    arg->doc_string = copy_string(((json_object *)map_get(arg_desc, "__doc__"))->content.data);
    
    char *arg_type = ((json_object *)map_get(arg_desc, "type"))->content.data;
    if (strcmp(arg_type, "string")) {
        arg->type = STRING_ARG;
    } else if (strcmp(arg_type, "integer")) {
        arg->type = INTEGER_ARG;
    } else if (strcmp(arg_type, "double")) {
        arg->type = DOUBLE_ARG;
    } else if (strcmp(arg_type, "binary")) {
        arg->type = BINARY_ARG;
    } else {
        arg_type = UNKNOWN_ARG;
    }

    json_object *is_mandatory_json_entry = map_get(arg_desc, "mandatory");
    arg->is_mandatory = 0;
    if (is_mandatory_json_entry != NULL) {
        char *is_mandatory = is_mandatory_json_entry->content.data;
        if (strcmp("true", is_mandatory) == 0) {
            arg->is_mandatory = 1;
        }
    }

    map_set(endpoint->args, copy_string(arg_name), arg);
}

void parse_endpoint(map *m, char *endpoint_name, service *svc) {

    printf("Parsing endpoint %s\n", endpoint_name);

    map *endpoint_desc = ((json_object *)map_get(m, endpoint_name))->content.object;

    service_endpoint *endpoint = (service_endpoint *)calloc(1, sizeof(service_endpoint));

    json_object *doc_string_json_entry = map_get(endpoint_desc, "__doc__");
    endpoint->doc_string = NULL;
    if (doc_string_json_entry != NULL) {
        endpoint->doc_string = copy_string(doc_string_json_entry->content.data);
    }

    endpoint->args = map_new(10);
    map_iter_keys(endpoint_desc, parse_endpoint_arg, endpoint);

    map_set(svc->endpoints, copy_string(endpoint_name), endpoint);
}

service *create_service_from_request(json_object *request) {

    service *svc = (service *)calloc(1, sizeof(service));

    // Get service data
    svc->name = copy_string(((json_object *)map_get(request->content.object, "name"))->content.data);
    svc->ip_address = copy_string(((json_object *)map_get(request->content.object, "addr"))->content.data);
    svc->port = atoi(((json_object *)map_get(request->content.object, "port"))->content.data);
    svc->doc_string = copy_string(((json_object *)map_get(request->content.object, "__doc__"))->content.data);
    
    // Get endpoints data
    map *endpoints_section = ((json_object *)map_get(request->content.object, "endpoints"))->content.object;
    svc->endpoints = map_new(10);

    map_iter_keys(endpoints_section, parse_endpoint, svc);

    return svc;
}

void endpoint_arg_dealloc(map *m, char *arg_name, void *args) {
    service_endpoint_arg *arg = map_get(m, arg_name);
    if (arg->name != NULL) free(arg->name);
    if (arg->doc_string != NULL) free(arg->doc_string);
}

void endpoint_dealloc(map *m, char *endpoint_name, void *args) {

    service_endpoint *endpoint = map_get(m, endpoint_name);
    if (endpoint->name != NULL) free(endpoint->name);
    if (endpoint->doc_string != NULL) free(endpoint->doc_string);

    map_iter_keys(endpoint->args, endpoint_arg_dealloc, NULL);
    map_destroy_dealloc(endpoint->args, 1, 1);
}

void service_dealloc(service *svc) {

    if (svc->name != NULL) free(svc->name);
    if (svc->doc_string != NULL) free(svc->doc_string);
    if (svc->ip_address != NULL) free(svc->ip_address);

    map_iter_keys(svc->endpoints, endpoint_dealloc, NULL);
    map_destroy_dealloc(svc->endpoints, 1, 1);

    free(svc);
}
