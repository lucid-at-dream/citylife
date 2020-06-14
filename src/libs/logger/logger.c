#include "logger.h"

#include <stdio.h>
#include <time.h>

void log_message(char *level, char *message, va_list args);

void info(char *message, ...) {
    va_list argptr;
    va_start(argptr, message);
    log_message(" INFO", message, argptr);
    va_end(argptr);
}

void error(char *message, ...) {
    va_list argptr;
    va_start(argptr, message);
    log_message("ERROR", message, argptr);
    va_end(argptr);
}

void debug(char *message, ...) {
#ifdef DEBUG
    va_list argptr;
    va_start(argptr, message);
    log_message("DEBUG", message, argptr);
    va_end(argptr);
#endif
}

void log_message(char *level, char *message, va_list args) {
    // Get time
    time_t rawtime;
    struct tm *info;
    char time_buffer[80];
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(time_buffer, 80, "%x %X", info);

    // Format the user message
    char user_message[1024];
    vsnprintf(user_message, 1024 * sizeof(char), message, args);

    // Format the message with logging enrichments
    char log_message[2048];
    snprintf(log_message, 2048 * sizeof(char), "[%s] %s - %s\n", level, time_buffer, user_message);

    // Print the message
    fprintf(stdout, log_message, args);
}
