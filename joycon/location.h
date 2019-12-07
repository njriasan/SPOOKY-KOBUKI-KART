#ifndef LOCATION_H
#define LOCATION_H

#include "connection.h"

#define SECOND_TO_MICROSECONDS 1000000

#define POWERUP_DELAY 5 * SECOND_TO_MICROSECONDS

#define HAZARD_DELAY 10 * SECOND_TO_MICROSECONDS

#define NUM_MAC_ADDRS 4

typedef struct {
  int server_fd;
  connection_node_t* node;
} sn_pair_t;

/*
 * Function that polls for a location information for the socket and
 * if found updates the location inside the node. This is intended to
 * be run as a separate thread for the duration of the connection so
 * it should exit if the read indicates a disconnect.
 */
void poll_for_location(sn_pair_t* pair);

/*
 * Function to display the current locations of all Kobukis. This is a debugging
 * function intended as an intermediate for later using a global location list
 * to make a powerup decision.
 */
void display_locations(connection_node_t* kobuki_list);
#endif

