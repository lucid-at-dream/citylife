#include "authenticator.h"
#include "logger/logger.h"
#include <stdlib.h>
#include <string.h>

authenticator *authenticator_new() {
    info("authenticator.c: creating a new authenticator struct");
    authenticator *auth = (authenticator *)calloc(1, sizeof(authenticator));
    auth->auth_table = map_new(16);
    return auth;
}

void authenticator_destroy(authenticator *auth) {
    info("authenticator.c: destroying authenticator struct");
    map_destroy_dealloc(auth->auth_table, 1, 1);
    free(auth);
}

result add_user(authenticator *auth, char *user, char *password) {
    result r;

    if (map_get(auth->auth_table, user) != NULL) {
        info("authenticator.c: attempt to add user %s, which already exists", user);
        r.result = AUTH_ERROR;
        r.message = "The username is already registered.";
        return r;
    }

    map_set(auth->auth_table, user, password);

    info("authenticator.c: user %s added with success", user);
    r.result = AUTH_SUCCESS;
    r.message = "User has been added with success.";
    return r;
}

result authenticate(authenticator *auth, char *user, char *token) {
    result r;

    char *stored_password = map_get(auth->auth_table, user);

    if (stored_password == NULL) {
        info("authenticator.c: attempt to authenticate user %s, which does not exist", user);
        r.result = AUTH_ERROR;
        r.message = "The provided username does not exist";
        return r;
    }

    int match = strcmp(stored_password, token);
    if (match == 0) {
        info("authenticator.c: successfully authenticated user %s", user);
        r.result = AUTH_SUCCESS;
        r.message = "User authentication was successful";
    } else {
        info("authenticator.c: attempt to authenticate user %s with a wrong password", user);
        r.result = AUTH_ERROR;
        r.message = "The wrong password has been provided for that username.";
    }

    return r;
}

result change_password(authenticator *auth, char *user, char *token, char *new_password) {
    result r;

    if (authenticate(auth, user, token).result == AUTH_SUCCESS) {
        map_set(auth->auth_table, user, new_password);
        info("authenticator.c: user %s's password successfully modified", user);
        r.result = AUTH_SUCCESS;
        r.message = "Successfully changed the user password.";
    } else {
        info("authenticator.c: Unable to change password for user %s because old password doesn't match", user);
        r.result = AUTH_ERROR;
        r.message = "The provided password does not match the one associated to this user.";
    }

    return r;
}

result del_user(authenticator *auth, char *user, char *token) {
    result r;

    if (authenticate(auth, user, token).result == AUTH_SUCCESS) {
        map_del(auth->auth_table, user);
        info("authenticator.c: user %s's successfully deleted", user);
        r.result = AUTH_SUCCESS;
        r.message = "User successfully deleted.";
    } else {
        info("authenticator.c: user %s was not deleted because the password did not match", user);
        r.result = AUTH_ERROR;
        r.message = "The provided password does not match the one associated to this user.";
    }

    return r;
}
