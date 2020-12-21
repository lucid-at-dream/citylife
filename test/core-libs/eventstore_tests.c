#include <stdlib.h>

#include <test.h>
#include <assert.h>

#include <eventschema.h>
#include <eventstore.h>

void cenas()
{
}

TEST_CASE(event_store_log_10_events, {
    event_schema *schema = event_schema_new();

    event_schema_add_field(schema, (field_config){ SEQUENTIAL, "event_id" });
    event_schema_add_field(schema, (field_config){ TIMESTAMP, "request_ts" });
    event_schema_add_field(schema, (field_config){ TIMESTAMP, "response_ts" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "service" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "service_path" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "request" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "response" });

    event_store *store = event_store_new(schema);

    // TODO: Add events to the event store

    event_store_del(store);
    // free(store);

    event_schema_del(schema);
    // free(schema);
})

TEST_SUITE(RUN_TEST("Test logging 10 events in the event store", event_store_log_10_events))
