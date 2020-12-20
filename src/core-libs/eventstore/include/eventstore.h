#pragma once

#include "event_structs.h"
#include "eventschema.h"
#include "eventindex.h"

/**
 * Allocates an event store based on the specified schema.
 */
event_store *event_store_new(event_schema *);

/**
 * Deallocates an event store.
 */
void event_store_del(event_store *);

/**
 * Add an event to the event store.
 */
void event_store_add_event(event_store *, event *);

/**
 * Merge the events matching some filter in order to compress the event store and save up some space.
 */
void event_store_compress_events(event_store *, event_filter *, char (*is_mergeable)(event *, event *),
                                 event *(*merge_routine)(event *, event *));
