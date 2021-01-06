#include "test.h"
#include "assert.h"

#include "map.h"

#include "json.h"

#include <stdlib.h>
#include <stdio.h>

TEST_CASE(test_parse_json_with_nested_dictionaries, {
    
    FILE *f = fopen("test/_resources/json/valid_json_dictionaries.json", "r");
    char buffer[10240] = {'\0'};
    fread(buffer, sizeof(char), 10240, f);
    fclose(f);

    map *parsed = parse_json(buffer);
    assertion_error += assert_not_null("Json correctly identified as a dictionary", parsed);

    assertion_error += assert_str_equals("String in first degree dictionary matches", "auth", map_get(parsed, "name"));
    assertion_error += assert_str_equals("String in first degree dictionary matches", "Authentication service", map_get(parsed, "__doc__"));

    map *methods = map_get(parsed, "methods");
    assertion_error += assert_not_null("Methods map exists", methods);

    map *login = map_get(methods, "login");
    assertion_error += assert_not_null("Login map exists", login);

    assertion_error += assert_str_equals(
        "String in third degree dictionary matches", 
        "Creates a new session token for the given user if the user/password is a match", 
        map_get(login, "__doc__")
    );

    map *user_param = map_get(login, "user");
    assert_not_null("user param exists", user_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(user_param, "type"), "string");
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(user_param, "__doc__"), "The username");

    map *pass_param = map_get(login, "pass");
    assert_not_null("pass param exists", pass_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(pass_param, "type"), "string");
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(pass_param, "__doc__"), "The password");

    map *logout = map_get(methods, "logout");
    assertion_error += assert_not_null("Logout map exists", logout);

    map *token_param = map_get(logout, "token");
    assert_not_null("token param exists", token_param);
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(token_param, "type"), "string");
    assertion_error += assert_str_equals("String in fourth degree dictionary matches", map_get(token_param, "__doc__"), "The session token");

    // Dealloc all memory (TODO: Create some manner of doing this without having to know the whole json before hand)
    map_destroy_dealloc(token_param, 1, 1);
    map_del_dealloc(logout, "token", 1, 0);
    map_destroy_dealloc(logout, 1, 1);

    map_destroy_dealloc(user_param, 1, 1);
    map_destroy_dealloc(pass_param, 1, 1);
    map_del_dealloc(login, "user", 1, 0);
    map_del_dealloc(login, "pass", 1, 0);
    map_destroy_dealloc(login, 1, 1);

    map_del_dealloc(methods, "login", 1, 0);
    map_del_dealloc(methods, "logout", 1, 0);
    map_destroy_dealloc(methods, 1, 1);
    
    map_del_dealloc(parsed, "methods", 1, 0);
    map_destroy_dealloc(parsed, 1, 1);

})

TEST_SUITE(
    RUN_TEST("Parse a Json with only dictionaries, nested.", test_parse_json_with_nested_dictionaries)
)
