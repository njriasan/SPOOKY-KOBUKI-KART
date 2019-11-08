#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hidapi/hidapi.h"
#include "read_joycon_input.h"


// Experimentation showed a msg size of 12 bytes per joycon input
#define MSG_SIZE 12

/*
 * TODO:
 *   1. Add code to spawn a server socket
 *   2. Pipe the server socket number to parent
 *   3. Send the data over the socket connection as it arrives
 */


/*
 * Highest level function for a process that handles all the outputs from the joycon.
 */
int handle_joycon(int write_pipe_fd, char *device_path) {
    int res;

    // Initialize the hidapi library
    res = hid_init();
    assert (!res);

    // open the actual device
    hid_device *actual_dev = hid_open_path (device_path);
    assert (actual_dev);
    // Load in each message
    unsigned char response[MSG_SIZE];
    int data_len;
    while((data_len = hid_read (actual_dev, response, MSG_SIZE)) > 0) {
        for (int i = 0; i < data_len; i++) {
            printf ("%x ", response[i]);
        }
        printf ("\n");
    }

    // Finalize the hidapi library
    res = hid_exit();
    assert (!res);

    // This program shouldn't return unless we disconnect
    return 1;
}
