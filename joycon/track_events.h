#ifndef TRACK_EVENTS_H
#define TRACK_EVENTS_H

#include "location.h"

#define NUM_POLYGON_POINTS 4

typedef location_t track_elem_t[NUM_POLYGON_POINTS];

extern track_elem_t mushroom_tile;

extern track_elem_t redshell_tile;

extern track_elem_t banana_tile;

#endif
