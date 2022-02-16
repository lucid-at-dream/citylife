#include "test.h"
#include "assert.h"

#include "service.h"

#include <stdlib.h>
#include <stdio.h>

TEST_CASE(test_new_service_from_valid_json, {
    FILE *f = fopen("test/_resources/gateway/valid_service_registration_request.json", "r");
    char buffer[10240] = { '\0' };
    fread(buffer, sizeof(char), 10240, f);
    fclose(f);

    service *svc = service_new_from_json(buffer);

    // TODO: Assertions

    service_dealloc(svc);
})

TEST_SUITE(RUN_TEST("Create a new service from a valid json service registration request.", &test_new_service_from_valid_json))
