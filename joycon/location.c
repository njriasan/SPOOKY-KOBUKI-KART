#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "location.h"

typedef struct {
    int32_t x_int;
    int32_t y_int;
    int32_t z_int;
} int_location_t;

/*
 * Function that polls for a location information for the socket and
 * if found updates the location inside the node. This is intended to
 * be run as a separate thread for the duration of the connection so
 * it should exit if the read indicates a disconnect.
 */
void poll_for_location(sn_pair_t *pair) {
    int server_fd = pair->server_fd;
    connection_node_t *node = pair->node;
    location_t location;
    int_location_t int_location;

    // Detach the thread so we don't have to handle cleanup
    pthread_detach(pthread_self());

    // Interval at which to poll. This is a heuristic decision about
    // how often we expect to get a location update. 
    const int polling_time = SECOND_TO_MICROSECONDS / 10;

    // Listen for an initial connection
    assert (server_fd != -1);
    assert (listen (server_fd, 1) >= 0);
    int connection_socket = accept (server_fd, NULL, NULL);
    assert (connection_socket != -1);

    // Set the socket to non-blocking
    int existing_flags = fcntl(connection_socket, F_GETFL, NULL);
    fcntl(connection_socket, F_SETFL, O_NONBLOCK | existing_flags);

    // Structs for timing
    struct timeval start_time;
    struct timeval end_time;
    while (1) {
        // Update the start of the loop
        gettimeofday (&start_time, NULL);


        // We will always read either nothing or
        // busy loop til we get a full 12 bytes to avoid
        // an inconsistent state.
        size_t bytes_read = 0;
        bool location_read = false;
        bool should_read = true;
        int read_amount;
        uint8_t msg_buffer[13];
        // Read for as long as we don't 12 bytes. Note we are assuming sizeof(location_t) == 12
        // because it should be (and we need it to be since we don't send extra 0s).
        while (should_read) {
            // Read from the socket. Assumes the read is non-blocking
            while ((read_amount = read(connection_socket, (void *) &msg_buffer, 
                            13 - (bytes_read % 13))) > 0) {
                if (bytes_read == 13) {
                    bytes_read = read_amount;
                } else {
                    bytes_read += read_amount;
                    if (bytes_read == 13) {
                        if (msg_buffer[0] == NO_SHELL_REQUEST) {
                            memcpy(&int_location, &msg_buffer[1], 12);
                            location_read = true;
                        } else if (msg_buffer[0] == REDSHELL_REQUEST || 
                                msg_buffer[0] == BLUESHELL_REQUEST) {
                           set_shell_request(node, msg_buffer[0]);
                        }
                    }
                }
            }
            // If we read 0 we have disconnected
            if (read_amount == 0) {
                free(pair);
                pthread_exit(NULL);
            // If we read -1 we have nothing to read. This should terminate
            // our read attempts unless we have a partial read completed.
            } else if (read_amount == -1 && (bytes_read % 13 == 0)) {
                should_read = false;
            }
        }
        // If we read any data update the location
        if (location_read) {
            location.x = (float) int_location.x_int;
            location.y = (float) int_location.y_int;
            location.z = (float) int_location.z_int;
            printf("Setting the location\n");
            set_location(node, &location);
        }

        // Check if you need to deliver an event
        uint8_t event_value = get_event_request_reset(node);
        if (event_value != NO_EVENT) {
           uint8_t write_result = 0;
           while ((write_result = write(connection_socket, &event_value, 1)) == -1) {
               printf("Non-blocking issue with socket\n");
           }
           if (write_result == 0) {
                free(pair);
                pthread_exit(NULL);
           }
        }

        // Update the end of the loop 
        gettimeofday(&end_time, NULL);

        // If there is any time left sleep for that amount of time
        int time_remaining = polling_time - 
            ((end_time.tv_sec - start_time.tv_sec) * SECOND_TO_MICROSECONDS
            + (end_time.tv_usec - start_time.tv_usec));

        if (time_remaining > 0) {
            usleep(time_remaining);
        }
    }
}

typedef struct {
    connection_node_t *kobuki_node;
    location_t location;
} kobuki_info_t;

/*
 * Helper function to select which kobuki is hit by a redshell and then hit
 * that Kobuki.
 */
void assign_blueshell(kobuki_info_t *in_play_kobukis, size_t num_kobukis, size_t requesting_index) {

}

/*
 * Helper function to shell which kobuki is hit by a blueshell and then hit
 * that Kobuki.
 */
void assign_blueshell(kobuki_info_t *in_play_kobukis, size_t num_kobukis, size_t requesting_index) {

}

/*
 * Helper function used to process any shell requests.
 */
void process_used_shells(kobuki_info_t *in_play_kobukis, size_t num_kobukis) {
    // Add code for processing if any kobukis use a powerup
    for (size_t i = 0; i < num_kobukis; i++) {
        connection_node_t *node = in_play_kobukis[i].kobuki_node;
        uint8_t shell_request = get_shell_request(node);
        if (shell_reqeuest == REDSHELL_REQUEST) {
            assign_blueshell(in_play_kobukis, num_kobukis, i);
        } else if (shell_request == BLUESHELL_REQUEST) {
            assign_blueshell(in_play_kobukis, num_kobukis, i); 
        }
    }
}

/*
 * Helper function used to assign powerups/hazards on the track to the closest Kobuki.
 */
void assign_track_events(kobuki_info_t *in_play_kobukis, size_t num_kobukis) {
    // Add code to determine if any kobukis are in range of the powerup/hazard
}

/*
 * Helper function used for creating the leaderboard.
 */
void generate_leaderboard(kobuki_info_t *in_play_kobukis, size_t num_kobukis) {
    // Fill in code to generate rankings based on the map layout
    for (size_t i = 0; i < num_kobukis; i++) {
        printf("Kobuki %s at location(x:%f, y:%f, z:%f)\n", in_play_kobukis[i],kobuki_node->readable_name,
                in_play_kobukis[i].location.x, in_play_kobukis[i].location.y, in_play_kobukis[i].location.z);
    }
}

/*
 * Function to display the current locations of all Kobukis. This is a debugging
 * function intended as an intermediate for later using a global location list
 * to make a powerup decision.
 */
void display_locations(connection_node_t *kobuki_list) {
    connection_node_t *kobuki;
    // We can never have more than NUM_MAC_ADDRS locations
    kobuki_info_t valid_kobukis[NUM_MAC_ADDRS];
    // Actual number of active kobukis
    size_t num_kobukis = 0;
    // First generate a list of locations
    for (kobuki = kobuki_list; kobuki != NULL; kobuki = kobuki->next) {
        if (get_location(kobuki, &valid_kobukis[num_kobukis].location)) {
            valid_kobukis[num_kobukis].kobuki_node = kobuki;
            num_kobukis += 1;
        }
    }
    // Now generate the leaderboard
    generate_leaderboard(valid_kobukis, num_kobukis);

    // Check to see if you should assign any powerups or hazards
    assign_track_events(valid_kobukis, num_kobukis);

    // Check to see if any shells have been activated
    process_used_shells(valid_kobukis, num_kobukis);
}
