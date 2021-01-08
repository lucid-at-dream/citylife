#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "ctest/test.h"
#include "ctest/assert.h"
#include "hashmap/map.h"
#include "authenticator.h"

char *new_string(char *);

char test_auth_add_user()
{
    authenticator *auth = authenticator_new();
    result auth_result = add_user(auth, "ze", "ze");
    authenticator_destroy(auth);

    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        return 1;
    }
    return 0;
}

char test_auth_add_same_user_twice()
{
    authenticator *auth = authenticator_new();

    char *ZE_NAME = "ze";
    char *ZE_PASS = "ze";
    char *ZE_PASS_OTHER = "manel";

    result auth_result = add_user(auth, ZE_NAME, ZE_PASS);
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = add_user(auth, ZE_NAME, ZE_PASS_OTHER);
    if (assert_int_equals("Adding a new user with a conflicting name results in error.", auth_result.result, AUTH_ERROR)) {
        authenticator_destroy(auth);
        return 1;
    }
    authenticator_destroy(auth);
    return 0;
}

char test_auth_do_auth()
{
    authenticator *auth = authenticator_new();

    result auth_result = add_user(auth, "ze", "ze");
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    authenticator_destroy(auth);
    return 0;
}

char test_auth_do_auth_wrong_password()
{
    authenticator *auth = authenticator_new();

    result auth_result = add_user(auth, "ze", "ze");
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "maria");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_ERROR)) {
        authenticator_destroy(auth);
        return 1;
    }

    authenticator_destroy(auth);
    return 0;
}

char test_auth_do_auth_change_password()
{
    authenticator *auth = authenticator_new();

    result auth_result = add_user(auth, "ze", "ze");
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    char *new_pass = "maria";

    auth_result = change_password(auth, "ze", "maria", new_pass);
    if (assert_int_equals("Password for user ze is changed with success.", auth_result.result, AUTH_ERROR)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = change_password(auth, "ze", "ze", new_pass);
    if (assert_int_equals("Password for user ze is changed with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "maria");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    authenticator_destroy(auth);
    return 0;
}

char test_auth_do_auth_delete_user()
{
    authenticator *auth = authenticator_new();

    result auth_result = add_user(auth, "ze", "ze");
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = del_user(auth, "ze", "ze");
    if (assert_int_equals("User ze is deleted with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("Authentication with deleted user fails.", auth_result.result, AUTH_ERROR)) {
        authenticator_destroy(auth);
        return 1;
    }

    authenticator_destroy(auth);
    return 0;
}

char test_auth_do_auth_delete_user_wrong_pass()
{
    authenticator *auth = authenticator_new();

    result auth_result = add_user(auth, "ze", "ze");
    if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = del_user(auth, "ze", "maria");
    if (assert_int_equals("User ze is deleted with success.", auth_result.result, AUTH_ERROR)) {
        authenticator_destroy(auth);
        return 1;
    }

    auth_result = authenticate(auth, "ze", "ze");
    if (assert_int_equals("Authentication with deleted user fails.", auth_result.result, AUTH_SUCCESS)) {
        authenticator_destroy(auth);
        return 1;
    }

    authenticator_destroy(auth);
    return 0;
}

test test_suite[] = { { "Add user Ze with password Ze to the authentication service", test_auth_add_user },
                      { "Try to add a new user with a conflicting name", test_auth_add_same_user_twice },
                      { "Create an user and authenticate with it", test_auth_do_auth },
                      { "Create an user and provide a wrong password for authentication", test_auth_do_auth_wrong_password },
                      { "Test changing the password of an existing user", test_auth_do_auth_change_password },
                      { "Test deleting an existing user", test_auth_do_auth_delete_user },
                      { "Test that existing user is not deleted if wrong pass is provided", test_auth_do_auth_delete_user_wrong_pass } };

int main(int argc, char **argv)
{
    suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}

char *new_string(char *string)
{
    char *ns = (char *)calloc(strlen(string) + 1, sizeof(char));
    sprintf(ns, "%s", string);
    return ns;
}
