#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "connection.h"

/*
 * Constructor for a joycon's node. Assumes that the wchar_t
 * is malloced and it now has ownership (and must free it).
 */
connection_node_t *create_node (wchar_t *joycon_mac_addr, char *buckler_mac_addr) {
    connection_node_t *node = malloc (sizeof (connection_node_t));
    assert (node);
    node->joycon_mac_addr = joycon_mac_addr;
    node->buckler_mac_addr = buckler_mac_addr;
    node->next = NULL;
    node->joycon_input_pid = -1;
    node->ble_output_pid = -1;
    node->is_valid_location = false;
    pthread_mutex_init(&node->location_lock, NULL);
    return node;
}

/*
 * Append node to the front of list in place.
 */
void append_to_front (connection_node_t **list, connection_node_t *node) {
    assert (list);
    assert (node);
    node->next = *list;
    *list = node;
}

/*
 * Removes a node from the list in place. Assumes that node is a shallow copy.
 */
void remove_node (connection_node_t **list, connection_node_t *node) {
    assert (list);
    assert (node);
    while (*list) {
        if (*list == node) {
            *list = node->next;
            node->next = NULL;
            return;
        } else {
            list = &(*list)->next;
        }
    }
}

/*
 * Helper function used to set the location inside of a connection node to the
 * value passed in by location_ptr.
 */
void set_location(connection_node_t *node, location_t* location_ptr) {
    pthread_mutex_lock(&node->location_lock);
    memcpy(&node->location, location_ptr, sizeof(location_t));
    node->is_valid_location = true;
    pthread_mutex_unlock(&node->location_lock);
}

/*
 * Helper function used to get the location inside of a connection node
 * and place the value inside of location_ptr (which is assumed to contain
 * allocated memory). Returns whether or not it was successfully able to
 * update a location.
 */
bool get_location(connection_node_t *node, location_t *location_ptr) {
    pthread_mutex_lock(&node->location_lock);
    bool valid_loc = node->is_valid_location;
    if (valid_loc) {
        memcpy(location_ptr, &node->location, sizeof(location_t));
    }
    pthread_mutex_unlock(&node->location_lock);
    return valid_loc; 
}

