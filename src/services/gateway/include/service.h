#pragma once

#include "map.h"
#include "list.h"

typedef enum _service_arg_type {
    BINARY_ARG,
    STRING_ARG,
    INTEGER_ARG,
    DOUBLE_ARG,
    UNKNOWN_ARG
} service_arg_type;

typedef struct _service_endpoint_arg {
    char *name;
    char *doc_string;
    char is_mandatory;
    service_arg_type type;
} service_endpoint_arg;

typedef struct _service_endpoint {
    char *name;
    char *doc_string;
    map *args;
} service_endpoint;

typedef struct _service {
    char *name;
    char *doc_string;
    char *ip_address;
    int port;
    map *endpoints;
} service;

service *service_new_from_json(char *json);
void service_dealloc(service *svc);
