#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <time.h>
#include <stdlib.h>

#include "server.h"
#include "../ctest/test.h"
#include "../ctest/assert.h"

char *echo_request_handler(char *request) {
  int size = strlen(request);
  char *response = (char *)calloc(size + 1, sizeof(char));
  strncpy(response, request, size);
  return response;
}

void *start_server_async(void *args) 
{ 
    socket_server *server = (socket_server *)args;
    server_start(server, echo_request_handler);
    return NULL;
}

char *send_message_to_server(socket_server *server, char *message) {
  
  int msg_size = strlen(message) + 1;
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  connect(sockfd, (struct sockaddr *)&server->server_address, sizeof(server->server_address));
  
  send(sockfd, message, msg_size * sizeof(char), 0);

  char *response = (char *)calloc(msg_size, sizeof(char));
  read(sockfd, response, msg_size * sizeof(char));
  
  close(sockfd);

  return response;
}

char test_connect_to_server() {
  srand(clock());
  int port = rand() % 20000 + 10000; // Random number between 10000 and 30000

  socket_server *server = server_new(port);

  // Start the server
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, start_server_async, server);

  // Wait for the server to start
  usleep(10000);

  // Signal server to stop
  server_stop(server);

  // Send message to server so it unblocks
  char *response = send_message_to_server(server, "ze");

  int assertion_error = assert_str_equals("The response should be the same as the data sent over.", response, "ze");
  
  if (response != NULL) {
    free(response);
  }

  if (assertion_error) {
    return 1;
  }

  // Wait for server to finish
  pthread_join(thread_id, NULL);

  free(server);

  return 0;
}

test test_suite[] = {
  {
    "Test that it is possible to exchange a message with the server", test_connect_to_server
  }
};

int main(int argc, char **argv) {
  suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  if (report.failures > 0) {
    return -1;
  }

  return 0;
}
