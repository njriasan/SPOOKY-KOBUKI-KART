#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>

/*
 * Struct for keeping track of information for each possible connection.
 */
typedef struct connection_node {
    // Mac address of connecting joycon
    wchar_t *joycon_mac_addr;
    // Mac address of buckler to connect to
    char *buckler_mac_addr;
    // pid of joycon used to wait on process
    pid_t joycon_input_pid;
    // pid of ble connection process. Used to kill
    // or wait on ble process
    pid_t ble_output_pid;
    // port used by the serve to connect the joycon input
    // and ble processes
    int32_t server_port;
    // ports used to pipe information back from the child joycon process
    // back to the controller to give its server node
    int pipe_fds[2];
    // Next info for keeping track of a linked list
    struct connection_node *next;
} connection_node_t;

/*
 * Constructor for a joycon's node. Assumes that the wchar_t
 * is malloced and it now has ownership (and must free it).
 */
connection_node_t *create_node (wchar_t *joycon_mac_addr, char *buckler_mac_addr);

/*
 * Append node to the front of list in place.
 */
void append_to_front (connection_node_t **list, connection_node_t *node);


/*
 * Removes a node from the list in place. Assumes that node is a shallow copy.
 */
void remove_node (connection_node_t **list, connection_node_t *node);

#endif
