#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "location.h"

/*
 * Function that polls for a location information for the socket and
 * if found updates the location inside the node. This is intended to
 * be run as a separate thread for the duration of the connection so
 * it should exit if the read indicates a disconnect.
 */
void poll_for_location(int socket_fd, connection_node_t *node) {
    location_t location;

    // Detach the thread so we don't have to handle cleanup
    pthread_detach(pthread_self());

    // Interval at which to poll. This is a heuristic decision about
    // how often we expect to get a location update. 
    const int polling_time = SECOND_TO_MICROSECONDS / 10;

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
        bool should_read = true;
        int read_amount;
        // Read for as long as we don't 12 bytes. Note we are assuming sizeof(location_t) == 12
        // because it should be (and we need it to be since we don't send extra 0s).
        while (should_read) {
            // Read from the socket. Assumes the read is non-blocking
            while ((read_amount = read(socket_fd, (void *) &location,
                            12 - (bytes_read % 12)))) {
                if (bytes_read == 12) {
                    bytes_read = read_amount;
                } else {
                    bytes_read += read_amount;
                }
            }
            // If we read 0 we have disconnected
            if (read_amount == 0) {
                pthread_exit(NULL);
            // If we read -1 we have nothing to read. This should terminate
            // our read attempts unless we have a partial read completed.
            } else if (read_amount == -1 && (bytes_read % 12 == 0)) {
                should_read = false;
            }
        }
        // If we read any data update the location
        if (bytes_read) {
            set_location(node, &location);
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

/*
 * Function to display the current locations of all Kobukis. This is a debugging
 * function intended as an intermediate for later using a global location list
 * to make a powerup decision.
 */
void display_locations(connection_node_t *kobuki_list) {
    connection_node_t *kobuki;
    location_t location;
    for (kobuki = kobuki_list; kobuki != NULL; kobuki = kobuki->next) {
        pthread_mutex_lock(&kobuki->location_lock);
        if (get_location(kobuki, &location)) {
            printf("Kobuki %s at location(x:%f, y:%f, z:%f)\n", kobuki->readable_name,
                    location.x, location.y, location.z);
        }
        pthread_mutex_unlock(&kobuki->location_lock);
    }
}
