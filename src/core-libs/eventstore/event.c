#include "event_structs.h"
#include "map.h"

#include <stdlib.h>
#include <string.h>

event *event_new() {
    event *e = (event *)malloc(sizeof(event));
    e->fields = map_new(10);
    return e;
}

void event_del(event *e) {
    map_destroy_dealloc(e->fields, 1, 1);
    free(e);
}

void event_set_field(event *e, char *field_name, void *value, size_t value_size) {
    
    // Copy key
    int field_name_len = strnlen(field_name, MAX_SCHEMA_FIELD_NAME_SIZE);
    char *key = (char *)calloc(field_name_len + 1, sizeof(char));
    memcpy(key, field_name, field_name_len * sizeof(char));

    // Copy value
    void *val = malloc(value_size);
    memcpy(val, value, value_size);
    
    map_set(e->fields, key, val);
}