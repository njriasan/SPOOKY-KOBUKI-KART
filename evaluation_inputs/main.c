#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

struct addrinfo* setup_address(char* port) {
  struct addrinfo* server;
  struct addrinfo hints;
  memset (&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  int success = getaddrinfo(NULL, port, &hints, &server);
  if (success != 0) {
    return NULL;
  }
  return server;
}

int setup_server_socket(struct addrinfo* server) {
  // Setup the addr socket
  bool connected = false;
  int sock       = -1;
  for (struct addrinfo* addr = server; addr != NULL && !connected; addr = addr->ai_next) {
    int sock = socket (addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock != -1) {
      int success = bind(sock, addr->ai_addr, addr->ai_addrlen);
      if (success == -1) {
        close (sock);
        sock = -1;
      } else {
        return sock;
      }
    }
  }
  return sock;
}

int spawn_server(char *port) {
    struct addrinfo* server = setup_address(port);
    if (server != NULL) {
      int server_fd = setup_server_socket (server);
      if (server_fd != -1) {
        return server_fd;
      }
    }
    return -1;
}

void transfer_controller_data(int socket_fd, unsigned char* data, unsigned int size) {
  int total_write = 0;
  int curr_write  = 0;

  while (total_write < size && (curr_write = write (socket_fd,
                                                    data + total_write, size - total_write)) > 0) {
    total_write += curr_write;
  }

}

int main (int argc, char *argv[]) {
  assert(argc == 3);
  char *port = argv[1];
  int send_interval = atoi(argv[2]);
  int server_fd = spawn_server(port);
  assert (server_fd != -1);
  assert (listen (server_fd, 1) >= 0);
  int connection_socket = accept (server_fd, NULL, NULL);
  assert (connection_socket != -1);
  uint8_t msg[12];
  memset(msg, 0, 12);
  struct timeval start_time;
  while (1) { 
    gettimeofday (&start_time, NULL);
    // Add in logic to print
    
    transfer_controller_data(connection_socket, msg, 12);
    sleep(send_interval);
  }
}
