#include <stdio.h>
#include "location.h"

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
