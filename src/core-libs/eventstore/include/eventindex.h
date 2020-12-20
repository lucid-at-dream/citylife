#pragma once

#include "event_structs.h"

/**
 * Creates a new index for events based on the given event configuration.
 */
event_index *event_index_new(event_store *, event_index_config);

/**
 * Deletes all resources associated with an event index.
 */
void event_index_del(event_index *);
