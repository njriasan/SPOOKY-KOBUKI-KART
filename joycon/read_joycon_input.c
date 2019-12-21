#include <assert.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "hidapi/hidapi.h"
#include "read_joycon_input.h"
#include "location.h"


// Experimentation showed a msg size of 12 bytes per joycon input
#define MSG_SIZE 12
#define MIN_PORT 10000
#define MAX_PORT 50000

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


/*
 *  Function that sends a port number across the pipe and closes the pipe.
 */
void send_port(int write_pipe_fd, int port_num) {
  char port[6];
  snprintf (port, 6, "%d", port_num);
  ssize_t data_sent  = 0;
  ssize_t curr_write = 0;
  while (data_sent < sizeof(int32_t) &&
         (curr_write = write (write_pipe_fd, ((char*)&port_num) + data_sent,
                              sizeof(int32_t) - data_sent)) > 0) {

    data_sent += curr_write;
  }
  close (write_pipe_fd);
}

/*
 *  function that spawns a server and then places the port number
 *  inside the pointer passed in.
 */
int spawn_server(int* port_ptr) {
  srand (time(0));
  while (true) {
    int32_t port_num = (rand () % (MAX_PORT - MIN_PORT)) + MIN_PORT;
    char port[6];
    snprintf (port, 6, "%d", port_num);
    struct addrinfo* server = setup_address(port);
    if (server != NULL) {
      int server_fd = setup_server_socket (server);
      if (server_fd != -1) {
        *port_ptr = port_num;
        return server_fd;
      }
    }
  }
  return -1;
}


/*
 * Helper function for reading all the data from one transmission at once
 * and placing it inside response, which has room for at least size bytes.
 */
void read_hid_data(hid_device* actual_dev, unsigned char* response, unsigned int size) {
  int total_data = 0;
  int curr_read  = 0;
  while (total_data < size && (curr_read = hid_read (actual_dev,
                                                     response + total_data, size - total_data)) > 0) {
    total_data += curr_read;
  }
  if (total_data == 0) {
    // fprintf (stderr, "Connection Lost\n");
    exit (1);
  }
}


void read_eval_data(int socket_fd, unsigned char* data, unsigned int size) {
  int total_read = 0;
  int curr_read  = 0;
  while (total_read < size && (curr_read = read (socket_fd,
                                                    data + total_read, size - total_read)) > 0) {
    total_read += curr_read;
  }
  if (total_read == 0) {
    exit (1);
  }

}

/*
 * Writes size bytes through the socket.
 */
void transfer_hid_data(int socket_fd, unsigned char* data, unsigned int size) {
  int total_write = 0;
  int curr_write  = 0;

  // for (int i = 0; i < size; i++) {
  //    printf("%x ", data[i]);
  // }
  // printf("\n");

  while (total_write < size && (curr_write = write (socket_fd,
                                                    data + total_write, size - total_write)) > 0) {
    total_write += curr_write;
  }

}

struct addrinfo* get_addrs (char* host, char* port) {
    struct addrinfo* addrs;
    struct addrinfo hints;
    memset(&hints, 0,  sizeof(struct sockaddr));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rv = getaddrinfo(host, port, &hints, &addrs);
    if (rv != 0) {
        return NULL;
    }
    return addrs;
}

int setup_client_socket (struct addrinfo *addrs) {
    // Setup the addr socket
    bool connected = false;
    int sock = -1;
    for (struct addrinfo *addr = addrs; addr != NULL && !connected; addr = addr->ai_next) {
        int sock = socket (addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock != -1) {
            int success = connect(sock, addr->ai_addr, addr->ai_addrlen);
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

/*
 * Highest level function for a process that handles all the outputs from the joycon.
 */
int handle_joycon(int write_pipe_fd, char* device_path, char *eval_port_num) {
  int res;
  int port_num = 0;
  char *str = malloc(100);
  strcpy(str, "joycon-input-files");
  strcat(str, eval_port_num);

  FILE *write_file = fopen(str, "w");
  // Spawn the server socket used for IPC with the BLE layer
  int server_fd = spawn_server (&port_num);
  send_port(write_pipe_fd, port_num);
  assert (server_fd != -1);
  assert (listen (server_fd, 1) >= 0);
  int connection_socket = accept (server_fd, NULL, NULL);
  assert (connection_socket != -1);

  // Initialize the hidapi library
  // res = hid_init();
  // assert (!res);

  // Establish a connection with the evaluation socket
  char *host = "127.0.0.1";
  // printf("%s\n", eval_port_num);
  int eval_socket = setup_client_socket(get_addrs(host, eval_port_num));

  // open the actual device
  // hid_device* actual_dev = hid_open_path (device_path);

  // Load in each message
  unsigned char response[MSG_SIZE];
  struct timeval start_time;
  while (true) {
    // read_hid_data (actual_dev, response, MSG_SIZE);
    read_eval_data (eval_socket, response, MSG_SIZE);
    gettimeofday (&start_time, NULL);
    // Add in logic to print
    uint64_t time_remaining = start_time.tv_sec * SECOND_TO_MICROSECONDS + start_time.tv_usec;
    fprintf(write_file, "%lu\n", time_remaining);
    transfer_hid_data (connection_socket, response, MSG_SIZE);
  }

  // Finalize the hidapi library
  res = hid_exit();
  assert (!res);

  // This program shouldn't return unless we disconnect
  return 1;
}

