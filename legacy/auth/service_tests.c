#include "ctest/assert.h"
#include "ctest/test.h"
#include "logger/logger.h"
#include "service.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int port;
pthread_t thread_id;

char *send_message_to_server(int port, char *message, char expect_response);

void *run_service_async(void *args)
{
    int port = *(int *)args;

    // Start the service
    service_run(port);

    return NULL;
}

char run_before_test()
{
    // Assign a random port number to the service
    srand(clock());
    port = rand() % 20000 + 10000;

    // Start the server
    pthread_create(&thread_id, NULL, run_service_async, &port);

    // Wait for the service to be ready. (250ms)
    usleep(250000);
    return 0;
}

char run_after_test()
{
    // Stop the service and wait for it to finish
    service_stop();
    send_message_to_server(port, "", 0);
    pthread_join(thread_id, NULL);
    info("Service stopped cleanly");

    // Delete allocated resources
    service_del();

    return 0;
}

char *send_message_to_server(int port, char *message, char expect_response)
{
    int msg_size = strlen(message) + 1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    send(sockfd, message, msg_size * sizeof(char), 0);

    char *response = NULL;
    if (expect_response)
    {
        response = (char *)calloc(1024, sizeof(char));
        read(sockfd, response, 1024 * sizeof(char));
    }

    close(sockfd);

    return response;
}

char test_create_and_authenticate_user()
{
    run_before_test();

    int assertion_error = 0;
    char *response, *message;

    message = "{"
              "\"action\":\"new\","
              "\"user\":\"ze\","
              "\"pass\":\"ze\""
              "}";
    response = send_message_to_server(port, message, 1);
    assertion_error += assert_substr_in("User added successfuly", "success", response);
    free(response);

    message = "{"
              "\"action\":\"auth\","
              "\"user\":\"ze\","
              "\"pass\":\"ze\""
              "}";
    response = send_message_to_server(port, message, 1);
    assertion_error += assert_substr_in("User authenticated successfuly", "success", response);
    free(response);

    run_after_test();
    return assertion_error;
}

test test_suite[] = { { "Test creating an user and authenticating with it", test_create_and_authenticate_user } };

int main(int argc, char **argv)
{
    suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

    if (report.failures > 0)
    {
        return -1;
    }

    return 0;
}
