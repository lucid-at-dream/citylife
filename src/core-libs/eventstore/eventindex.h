#pragma once
#include "eventstore.h"

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
 * Creates a new index for events based on the given event configuration.
 */
event_index *event_index_new(event_store *, event_index_config);

/**
 * Deletes all resources associated with an event index.
 */
void event_index_del(event_index *);
