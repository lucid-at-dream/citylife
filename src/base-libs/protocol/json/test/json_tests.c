#include "test.h"
#include "assert.h"

#include "map.h"

#include "json.h"

#include <stdlib.h>
#include <stdio.h>

TEST_CASE(test_parse_json_with_nested_dictionaries, {
    FILE *f = fopen("test/_resources/json/valid_json_dictionaries.json", "r");
    char buffer[10240] = { '\0' };
    fread(buffer, sizeof(char), 10240, f);
    fclose(f);

    json_object *parsed_json = parse_json(buffer);

    map *parsed = parsed_json->content.object;
    assertion_error += assert_not_null("Json correctly identified as a dictionary", parsed);

    assertion_error += assert_str_equals("String in first degree dictionary matches", ((json_object *)map_get(parsed, "name"))->content.data, "auth");
    assertion_error +=
            assert_str_equals("String in first degree dictionary matches", ((json_object *)map_get(parsed, "__doc__"))->content.data, "Authentication service");

    map *methods = ((json_object *)map_get(parsed, "methods"))->content.object;
    assertion_error += assert_not_null("Methods map exists", methods);

    map *login = ((json_object *)map_get(methods, "login"))->content.object;
    assertion_error += assert_not_null("Login map exists", login);

    assertion_error +=
            assert_str_equals("String in third degree dictionary matches", "Creates a new session token for the given user if the user/password is a match",
                              ((json_object *)map_get(login, "__doc__"))->content.data);

    map *user_param = ((json_object *)map_get(login, "user"))->content.object;
    assert_not_null("user param exists", user_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(user_param, "type"))->content.data, "string");
    assertion_error +=
            assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(user_param, "__doc__"))->content.data, "The username");

    map *pass_param = ((json_object *)map_get(login, "pass"))->content.object;
    assert_not_null("pass param exists", pass_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(pass_param, "type"))->content.data, "string");
    assertion_error +=
            assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(pass_param, "__doc__"))->content.data, "The password");

    map *logout = ((json_object *)map_get(methods, "logout"))->content.object;
    assertion_error += assert_not_null("Logout map exists", logout);

    map *token_param = ((json_object *)map_get(logout, "token"))->content.object;
    assert_not_null("token param exists", token_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(token_param, "type"))->content.data, "string");
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", ((json_object *)map_get(token_param, "__doc__"))->content.data,
                                         "The session token");

    // Dealloc all memory (TODO: Create some manner of doing this without having to know the whole json before hand)
    json_dealloc(parsed_json);
})

TEST_SUITE(RUN_TEST("Parse a Json with only dictionaries, nested.", &test_parse_json_with_nested_dictionaries))
