#include "eventstore.h"

#include <stdlib.h>

event_store *event_store_new(event_schema *schema)
{
    event_store *store = (event_store *)malloc(sizeof(event_store));
    store->events = list_new();
    return store;
}

void delete_event(void **e)
{
    event_del(*e);
}

void event_store_del(event_store *store)
{
    list_foreach(store->events, delete_event);
    list_destroy(store->events);
    free(store);
}

void event_store_add_event(event_store *store, event *e)
{
    list_append(store->events, e);
}

void event_store_compress_events(event_store *store, event_filter *filter, char (*is_mergeable)(event *, event *), event *(*merge_routine)(event *, event *))
{
}