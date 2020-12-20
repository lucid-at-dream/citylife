#include "eventstore.h"

#include <stdlib.h>

event_store *event_store_new(event_schema *schema) {
    return NULL;
}

void event_store_del(event_store *store) {
    
}

void event_store_add_event(event_store *store, event *event) {
    
}

void event_store_compress_events(event_store *store, event_filter *filter, char (*is_mergeable)(event *, event *),
                                 event *(*merge_routine)(event *, event *)) {

}