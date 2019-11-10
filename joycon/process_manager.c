#include <assert.h>
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

#define SECOND_TO_MICROSECONDS 1000000

// List of all the mac addresses for possible joy con pairings
const char *joycon_mac_addrs[NUM_MAC_ADDRS] = {"04:03:d6:7b:59:ca", "04:03:d6:7a:b8:75"};

// List of all MAC addresses for the bucklers (replace me later)
const char *buckler_mac_addrs[NUM_MAC_ADDRS] = {"04:03:d6:7b:59:ca", "04:03:d6:7a:b8:75"};

static connection_node_t *unprocessed_macs = NULL;
static connection_node_t *processed_macs = NULL;

/*
 * Helper function that takes in an array of char *s and converts each to a wchar_t*
 */
static wchar_t **convert_to_wide_strings (const char **joycon_mac_addrs, size_t num_strings) {
    wchar_t **wide_joycon_mac_addrs = malloc (sizeof (wchar_t *) * num_strings);
    assert (wide_joycon_mac_addrs);
    for (int i = 0; i < num_strings; i++) {
        wide_joycon_mac_addrs[i] = malloc((strlen (joycon_mac_addrs[i]) + 1) * sizeof(wchar_t));
        assert (wide_joycon_mac_addrs[i]);
        mbsrtowcs (wide_joycon_mac_addrs[i], &joycon_mac_addrs[i], strlen(joycon_mac_addrs[i]) + 1, NULL);
    }
    return wide_joycon_mac_addrs;
}

int main(int argc, char* argv[])
{
    if (argc != 1) {
        fprintf (stderr, "Usage: ./controller\n");
        exit (1);
    }
    int res;
    // Initialize all wide strings
    wchar_t **wide_joycon_mac_addrs = convert_to_wide_strings (joycon_mac_addrs, NUM_MAC_ADDRS);

    // Initialize unprocessed_macs
    for (int i = 0; i < NUM_MAC_ADDRS; i++) {
        // Create a node
        connection_node_t *node = create_node (wide_joycon_mac_addrs[i], buckler_mac_addrs[i]);
        // Add the node to the unprocessed list
        append_to_front (&unprocessed_macs, node);
    }
    // Remove the original list of macs
    free (wide_joycon_mac_addrs);

	// Initialize the hidapi library
    res = hid_init();
    assert (!res);

    while (true) {
        struct timeval start_time;
        struct timeval end_time;
        gettimeofday (&start_time, NULL);
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
                        // Close the unneeded read fd
                        close (node->pipe_fds[0]);
                        // Call the joycon handler
                        handle_joycon (node->pipe_fds[1], device->path);
                        perror ("Connection lost");
                        exit (1);
                    } else if (node->joycon_input_pid > 0) {
                        // Close the write portion of the pipe
                        close (node->pipe_fds[1]);
                        printf ("Forked a new process.\n");
                        // TODO: Add code to move this into a new thread
                        char *server_num_ptr = (char *) &node->server_port;
                        int read_size = 0;
                        int curr_read = 0;
                        while (read_size < sizeof (int32_t) && 
                                (curr_read = read (node->pipe_fds[0], 
                                                   server_num_ptr + read_size, 
                                                   sizeof(int32_t) - read_size) > 0)) {
                            read_size += curr_read;
                        }
                        close (node->pipe_fds[0]);
                        node->ble_output_pid = fork ();
                        if (node->ble_output_pid == 0) {
                            // Spawned child process
                            char *python_path = "python3";
                            char *args[5];
                            args[0] = python_path;
                            args[1] = "../send_pkt.py";
                            args[2] = node->buckler_mac_addr;
                            args[3] = malloc (sizeof(char) * 6);
                            snprintf (args[3], 6, "%d\n", node->server_port);
                            args[4] = NULL;
                            execvp (python_path, args);
                            perror ("Exec failed.");
                        } else if (node->ble_output_pid < 0) {
                            perror ("Unable to fork process");
                            break;
                        }

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

        // Iterate through the nodes and check all of the joycon_input_pids to determine
        // if any of the processes have exited. If so clean up that process and queue that
        // mac address up again as a possible mac address to check if a connection has been
        // established.
        connection_node_t *node = processed_macs;
        
        // Note we can't use a for loop here because it modify the linked list.
        while (node != NULL) {
            pid_t res = waitpid (node->joycon_input_pid, NULL, WNOHANG);
            /* A child process has exited. */
            if (res > 0) {
                /* kill off the python client as well. */
                kill (node->ble_output_pid, SIGKILL);
                waitpid (node->ble_output_pid, NULL, 0);
                connection_node_t *new_node = node->next;
                remove_node (&processed_macs, node);
                append_to_front (&unprocessed_macs, node); 
                node = new_node;
            } else {
                // If the client has exited instead restart both processes
                pid_t res = waitpid (node->ble_output_pid, NULL, WNOHANG);
                /* A child process has exited. */
                if (res > 0) {
                    /* kill off the python client as well. */
                    kill (node->joycon_input_pid, SIGKILL);
                    waitpid (node->joycon_input_pid, NULL, 0);
                    connection_node_t *new_node = node->next;
                    remove_node (&processed_macs, node);
                    append_to_front (&unprocessed_macs, node); 
                    node = new_node;
                } else {
                    node = node->next;
                }
            }
        }

        // We want to poll once every second for connections so sleep in the remaining time
        gettimeofday (&end_time, NULL);
        // Calculate how many microseconds are left for operations every second
        // and sleep if any time is remaining
        int time_remaining = SECOND_TO_MICROSECONDS - 
            ((end_time.tv_sec - start_time.tv_sec) * SECOND_TO_MICROSECONDS
            + (end_time.tv_usec - start_time.tv_usec));
        if (time_remaining > 0) {
            usleep (time_remaining);
        }
    }

	// Finalize the hidapi library
	res = hid_exit();
    assert (!res);

	return 1;
}
