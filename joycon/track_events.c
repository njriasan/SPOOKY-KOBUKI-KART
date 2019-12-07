#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "track_events.h"


location_t mushroom_tile = {6747.000000, -161.000000, 1100.000000};

location_t redshell_tile = {4454.000000, -677.000000, 1570.000000};

location_t banana_tile = {7637.000000, -37.000000, 1574.000000};



/*
 * Helper function to calculate the euclidean distance between two locations.
 */
double get_euclidean_distance(location_t *l1, location_t *l2) {
    double result = sqrt(pow(l1->x - l2->x, 2.0) + pow(l1->y - l2->y, 2.0) + pow(l1->z - l2->z, 2.0));
    printf("Result is %lf\n", result);
    return result;
}
