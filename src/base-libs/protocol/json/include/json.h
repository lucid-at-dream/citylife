#pragma once

#include "list.h"
#include "map.h"
#include <stdlib.h>
#include <string.h>

typedef enum _json_type
{
    JSON_ERROR,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_PRIMITIVE
} json_type;

typedef union _json_object_content
{
    map *object;
    list *array;
    char *data;
} json_object_content;

typedef struct _json_object
{
    json_type type;
    json_object_content content;
} json_object;

json_object *parse_json(char *json);
void json_dealloc(json_object *json);
