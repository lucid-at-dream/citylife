#pragma once

#include "map.h"

typedef struct _service {
    char *ip_address;
    int port;
    map *endpoints;
} service;

void *parse_request(char *request);
