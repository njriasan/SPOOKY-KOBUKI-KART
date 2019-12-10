#ifndef LOCATION_H
#define LOCATION_H

#include "connection.h"

#define SECOND_TO_MICROSECONDS 1000000

/*
 * Function that polls for a location information for the socket and
 * if found updates the location inside the node. This is intended to
 * be run as a separate thread for the duration of the connection so
 * it should exit if the read indicates a disconnect.
 */
void poll_for_location(int socket_fd, connection_node_t *node);

/*
 * Function to display the current locations of all Kobukis. This is a debugging
 * function intended as an intermediate for later using a global location list
 * to make a powerup decision.
 */
void display_locations(connection_node_t *kobuki_list);
#endif
