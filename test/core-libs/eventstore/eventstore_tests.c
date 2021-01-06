#include <stdlib.h>

#include <test.h>
#include <assert.h>

#include <event.h>
#include <eventschema.h>
#include <eventstore.h>

void cenas()
{
}

TEST_CASE(event_store_log_one_event, {
    event_schema *schema = event_schema_new();

    event_schema_add_field(schema, (field_config){ SEQUENTIAL, "event_id" });
    event_schema_add_field(schema, (field_config){ TIMESTAMP, "request_ts" });
    event_schema_add_field(schema, (field_config){ TIMESTAMP, "response_ts" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "service" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "service_path" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "request" });
    event_schema_add_field(schema, (field_config){ VARCHAR, "response" });

    event_store *store = event_store_new(schema);

    event *e = event_new();
    event_set_field(e, "service", "auth", sizeof(char) * 5);

    event_store_add_event(store, e);

    event_store_del(store);
    event_schema_del(schema);
})

TEST_SUITE(RUN_TEST("Test logging 10 events in the event store", event_store_log_one_event))
