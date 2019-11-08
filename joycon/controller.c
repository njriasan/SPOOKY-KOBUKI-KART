#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>
#include "connection.h"
#include "hidapi/hidapi.h"
#include "read_joycon_input.h"

/*
 * Program dedicated to work as the coordinating controller process
 * for each bluetooth connection process, as well as client connection code.
 */

#define NUM_MAC_ADDRS 2

// List of all the mac addresses for possible joy con pairings
const char *mac_addrs[NUM_MAC_ADDRS] = {"04:03:d6:7b:59:ca", "04:03:d6:7a:b8:75"};

static connection_node_t *unprocessed_macs = NULL;
static connection_node_t *processed_macs = NULL;

/*
 * Helper function that takes in an array of char *s and converts each to a wchar_t*
 */
static wchar_t **convert_to_wide_strings (const char **mac_addrs, size_t num_strings) {
    wchar_t **wide_mac_addrs = malloc (sizeof (wchar_t *) * num_strings);
    assert (wide_mac_addrs);
    for (int i = 0; i < num_strings; i++) {
        wide_mac_addrs[i] = malloc((strlen (mac_addrs[i]) + 1) * sizeof(wchar_t));
        assert (wide_mac_addrs[i]);
        mbsrtowcs (wide_mac_addrs[i], &mac_addrs[i], strlen(mac_addrs[i]) + 1, NULL);
    }
    return wide_mac_addrs;
}

int main(int argc, char* argv[])
{
    if (argc != 1) {
        perror ("Usage: ./controller");
        exit (1);
    }
    int res;
    // Initialize all wide strings
    wchar_t **wide_mac_addrs = convert_to_wide_strings (mac_addrs, NUM_MAC_ADDRS);

    // Initialize unprocessed_macs
    for (int i = 0; i < NUM_MAC_ADDRS; i++) {
        // Create a node
        connection_node_t *node = create_node (wide_mac_addrs[i]);
        // Add the node to the unprocessed list
        append_to_front (&unprocessed_macs, node);
    }
    // Remove the original list of macs
    free (wide_mac_addrs);

	// Initialize the hidapi library
    res = hid_init();
    assert (!res);

    while (true) {
        // Find all paired devices
        struct hid_device_info *devices = hid_enumerate (0, 0);
        
        struct hid_device_info *device = devices;
        // Iterate through all the devices looking for our mac addrs
        while (device != NULL) {
            // Check all mac addrs in the list
            for (connection_node_t *node = unprocessed_macs; node != NULL; node = node->next) {
                if (!wcscmp (device->serial_number, node->joycon_mac_addr)) {

                    // Call the pipe syscall to pass back the server
                    pipe (node->pipe_fds);
                    // Fork to spawn a separate process
                    node->joycon_input_pid = fork ();
                    if (node->joycon_input_pid == 0) {
                        // Spawned child process
                        int write_fd = node->pipe_fds[1];
                        char *path = device->path;
                        handle_joycon (write_fd, path);
                        perror ("Connection lost");
                        exit (1);
                    } else if (node->joycon_input_pid > 0) {
                        // Close the write portion of the pipe
                        close (node->pipe_fds[1]);
                        printf ("Forked a new process.\n");
                        // TODO: Add code to handle reading the server and spawning the new process

                    } else {
                        // If we error try again later
                        perror ("Unable to fork process");
                        break;
                    }
                    // Swap a node from being as treated as unprocessed to processed
                    remove_node (&unprocessed_macs, node);
                    append_to_front (&processed_macs, node);

                    // Move to the next device as we have already paired this device
                    break;
                }
            }
            device = device->next;
        }
        if (devices != NULL) {
            // Free the malloced data
            hid_free_enumeration (devices);
        }

        // Add code to check if any processes exited
        connection_node_t *node = processed_macs; 
        while (node != NULL) {
            pid_t res = waitpid (node->joycon_input_pid, NULL, WNOHANG);
            /* A child process has exited. */
            if (res > 0) {
                connection_node_t *new_node = node->next;    
                remove_node (&processed_macs, node);
                append_to_front (&unprocessed_macs, node); 
                node = new_node;
            } else {
                node = node->next;
            }
        }

        // We want to poll once every second for connections so sleep in the remaining time
        // FIXME to account for time spent 
        sleep (1);
    }

	// Finalize the hidapi library
	res = hid_exit();
    assert (!res);

	return 1;
}
