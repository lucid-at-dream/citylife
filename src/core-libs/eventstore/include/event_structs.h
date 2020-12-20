#pragma once

#include "datatypes.h"

/**
 * Defines the structure of an event store.
 */
typedef struct _event_store event_store;

/**
 * Defines a filter over events.
 */
typedef struct _event_filter event_filter;

/**
 * Defines the structure of an event index.
 */
typedef struct _event_index event_index;

/**
 * Defines the configuration of an event index.
 */
typedef struct _event_index_config event_index_config;

/**
 * Defines the structure of an event.
 */
typedef struct _event event;

typedef enum {
    UNIQ = 1 << 0
} field_constraints;

typedef struct _field_config {
    data_type type;
    char *name;
} field_config;

/**
 * Defines the schema of an event.
 */
typedef struct _event_schema event_schema;