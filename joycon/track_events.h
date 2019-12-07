#ifndef TRACK_EVENTS_H
#define TRACK_EVENTS_H

#include "location.h"

extern location_t mushroom_tile;

extern location_t redshell_tile;

extern location_t banana_tile;

#define ELEM_RADIUS 500.0

/*
 * Helper function to calculate the euclidean distance between two locations.
 */
double get_euclidean_distance(location_t *l1, location_t *l2);

#endif
