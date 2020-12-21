#pragma once

#include "datatypes.h"
#include "list.h"
#include "map.h"

#define MAX_SCHEMA_FIELD_NAME_SIZE 1024

/**
 * TODO: Defines the structure of an event store.
 */
typedef struct _event_store event_store;

/**
 * TODO: Defines a filter over events.
 */
typedef struct _event_filter event_filter;

/**
 * TODO: Defines the structure of an event index.
 */
typedef struct _event_index event_index;

/**
 * TODO: Defines the configuration of an event index.
 */
typedef struct _event_index_config event_index_config;

/**
 * Defines the structure of an event.
 */
typedef struct _event {
    map *fields;
} event;

/**
 * Defines a field in the schema.
 */
typedef struct _field_config {
    data_type type;
    char *name;
} field_config;


/**
 * Defines the schema of an event.
 */
typedef struct _event_schema {
    list *fields;
} event_schema;