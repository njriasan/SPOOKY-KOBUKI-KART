#include <assert.h>
#include <stdlib.h>
#include "connection.h"

/*
 * Constructor for a joycon's node. Assumes that the wchar_t
 * is malloced and it now has ownership (and must free it).
 */
connection_node_t *create_node (wchar_t *joycon_mac_addr) {
    connection_node_t *node = malloc (sizeof (connection_node_t));
    assert (node);
    node->joycon_mac_addr = joycon_mac_addr;
    node->next = NULL;
    node->joycon_input_pid = -1;
    node->ble_output_pid = -1;
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
