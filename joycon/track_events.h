#ifndef TRACK_EVENTS_H
#define TRACK_EVENTS_H

#include "location.h"

#define NUM_POLYGON_POINTS 4

typedef struct {

    // Identifier used to denote a particular track_elem. This should be a unique integer
    // between 0 and 7 (so we support up to 8 track events).
    uint8_t uuid;

    // Points the define the exterior vertices for defining the area of the polygon.
    location_t vertices[NUM_POLYGON_POINTS];
} track_elem_t;

#define NUM_POWERUPS 2

extern track_elem_t powerup_arr[NUM_POWERUPS];

#define NUM_HAZARDS 1

extern track_elem_t hazard_arr[NUM_HAZARDS];

/*
 * Helper function to verify that all uuids are unique and that all are < 8.
 */
void validate_uuids();

#endif
