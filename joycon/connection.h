#ifndef CONNECTION_H
#define CONNECTION_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

/*
 * Struct used for tracking location information for each node.
 */
typedef struct {
    float x;
    float y;
    float z;
} location_t;

/*
 * Struct for keeping track of information for each possible connection.
 */
typedef struct connection_node {
    // Mac address of connecting joycon
    wchar_t *joycon_mac_addr;
    // Mac address of buckler to connect to
    char *buckler_mac_addr;
    // Readable name for leaderboards/debugging
    char *readable_name;
    // pid of joycon used to wait on process
    pid_t joycon_input_pid;
    // pid of ble connection process. Used to kill
    // or wait on ble process
    pid_t ble_output_pid;
    // port used by the server to connect the joycon input
    // and ble processes
    int32_t controller_server_port;
    // port used by the server to transmit location information
    // to the process_manager as well as by the process_manager
    // for powerup and hazard info.
    int32_t location_server_port;
    // ports used to pipe information back from the child joycon process
    // back to the controller to give its server node
    int pipe_fds[2];
    // Current location of the kobuki
    location_t location;
    // Boolean to indicate if the node has a valid location. This should
    // only be false on startup or when a node has disconnected
    bool is_valid_location;
    // Lock for when multiple thread need to read a location value
    pthread_mutex_t location_lock;
    // Memory allocated for the thread that will poll for location updates.
    pthread_t thread;
    // Value used for communciating a shell request from the kobuki
    uint8_t shell_request;
    // Value used to communicate the acquisition of a powerup or hazard
    // to the kobuki
    uint8_t event_triggered;
    // Next info for keeping track of a linked list
    struct connection_node *next;
} connection_node_t;

// Grouping of values used for the shell request
#define NO_SHELL_REQUEST 0
#define REDSHELL_REQUEST 1
#define BLUESHELL_REQUEST 2

// Grouping of values used for the triggered event
#define NO_EVENT 0
#define MUSHROOM_POWERUP 1
#define REDSHELL_POWERUP 2
#define BLUESHELL_POWERUP 3
#define BANANA_HAZARD 4
#define REDSHELL_HAZARD 5
#define BLUESHELL_HAZARD 6

/*
 * Constructor for a joycon's node. Assumes that the wchar_t
 * is malloced and it now has ownership (and must free it).
 */
connection_node_t *create_node (wchar_t *joycon_mac_addr, char *buckler_mac_addr, char *readable_name);

/*
 * Append node to the front of list in place.
 */
void append_to_front (connection_node_t **list, connection_node_t *node);


/*
 * Removes a node from the list in place. Assumes that node is a shallow copy.
 */
void remove_node (connection_node_t **list, connection_node_t *node);

/*
 * Helper function used to set the location inside of a connection node.
 */
void set_location(connection_node_t *node, location_t* location_ptr);

/*
 * Helper function used to get the location inside of a connection node
 * and place the value inside of location_ptr (which is assumed to contain
 * allocated memory). Returns whether or not it was successfully able to
 * update a location.
 */
bool get_location(connection_node_t *node, location_t *location_ptr);

/*
 * Helper function used to set a shell request.
 */
void set_shell_request(connection_node_t *node, uint8_t request_value);

/*
 * Helper function used to get a shell request.
 */
uint8_t get_shell_request(connection_node_t *node);

/*
 * Helper function used to set an event for notifying the kobuki.
 */
void set_event(connection_node_t *node, uint8_t event_value);

/*
 * Helper function used to get an event for notifying the kobuki.
 */
uint8_t get_event_request(connection_node_t *node);
#endif
