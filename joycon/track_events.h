#ifndef TRACK_EVENTS_H
#define TRACK_EVENTS_H

#include "location.h"

typedef struct {
  location_t upper_left;
  location_t upper_right;
  location_t bottom_left;
  location_t bottom_right;
} track_elem_t;

extern track_elem_t mushroom_tile;
extern double mushroom_area;

extern track_elem_t redshell_tile;
extern double redshell_area;

extern track_elem_t banana_tile;
extern double banana_area;

/*
 * Helper function to determine if a point is within a rectangle.
 */
bool point_within_rectangle(location_t *point, track_elem_t *rect, double rect_area);

/*
 * Helper function to determine the area of a rectangle.
 */
double area_rectangle(track_elem_t *rect_tile);

/*
 * Helper function for the area of a triangle.
 *
 * Algorithm from https://math.stackexchange.com/questions/128991/how-to-calculate-the-area-of-a-3d-triangle
 */

double area_triangle(location_t *a, location_t *b, location_t *c);
/*
 * Helper function to calculate the euclidean distance between two locations.
 */
double get_euclidean_distance(location_t *l1, location_t *l2);

#endif
