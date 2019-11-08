#include <assert.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hidapi/hidapi.h"
#include "read_joycon_input.h"


// Experimentation showed a msg size of 12 bytes per joycon input
#define MSG_SIZE 12
#define MIN_PORT 10000
#define MAX_PORT 50000

struct addrinfo *setup_address (char *port) {
    struct addrinfo *server;
    struct addrinfo hints;
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int success = getaddrinfo(NULL, port, &hints, &server);
    if (success != 0) {
        return NULL;
    }
    return server;
}

int setup_server_socket (struct addrinfo *server) {
    // Setup the addr socket
    bool connected = false;
    int sock = -1;
    for (struct addrinfo *addr = server; addr != NULL && !connected; addr = addr->ai_next) {
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
 *  Function that spawns a server and then writes the port to the pipe
 *  specified, closing the pipe.
 */
int spawn_server (int write_pipe_fd) {
    while (true) {
        int32_t port_num = (rand () % (MAX_PORT - MIN_PORT)) + MIN_PORT;
        char port[6];
        snprintf (port, 6, "%d", port_num);
        struct addrinfo *server = setup_address(port);
        if (server != NULL) {
            int server_fd = setup_server_socket (server);
            if (server_fd != -1) {
                ssize_t data_sent = 0;
                ssize_t curr_write = 0;
                while (data_sent < sizeof(int32_t) && 
                        (curr_write = write (write_pipe_fd, ((char *) &port_num) + data_sent, 
                                             sizeof (int32_t) - data_sent)) > 0) {

                    data_sent += curr_write;
                }
                close (write_pipe_fd);
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
void read_hid_data (hid_device *actual_dev, unsigned char *response, unsigned int size) {
    int total_data = 0;
    int curr_read = 0;
    while(total_data < size && (curr_read = hid_read (actual_dev, 
                    response + total_data, size - total_data)) > 0) {
        total_data += curr_read;
    }
}


/*
 * Writes size bytes through the socket.
 */
void transfer_hid_data (int socket_fd, unsigned char *data, unsigned int size) {
    int total_write = 0;
    int curr_write = 0;
    while(total_write < size && (curr_write = write (socket_fd, 
                    data + total_write, size - total_write)) > 0) {
        total_write += curr_write;
    }

}

/*
 * Highest level function for a process that handles all the outputs from the joycon.
 */
int handle_joycon(int write_pipe_fd, char *device_path) {
    int res;

    // Spawn the server socket used for IPC with the BLE layer
    int server_fd = spawn_server (write_pipe_fd);
    assert (server_fd != -1);
    assert (listen (server_fd, 1) >= 0);
    int connection_socket = accept (server_fd, NULL, NULL);
    assert (connection_socket != -1);

    // Initialize the hidapi library
    res = hid_init();
    assert (!res);

    // open the actual device
    hid_device *actual_dev = hid_open_path (device_path);

    assert (actual_dev);
    // Load in each message
    unsigned char response[MSG_SIZE];
    while (true) {
        read_hid_data (actual_dev, response, MSG_SIZE);
        transfer_hid_data (connection_socket, response, MSG_SIZE);
    }

    // Finalize the hidapi library
    res = hid_exit();
    assert (!res);

    // This program shouldn't return unless we disconnect
    return 1;
}
