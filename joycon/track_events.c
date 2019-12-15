#include <assert.h>
#include <math.h>
#include "track_events.h"


track_elem_t mushroom_tile = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
double mushroom_area = 0.0;

track_elem_t redshell_tile = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
double redshell_area = 0.0;

track_elem_t banana_tile = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
double banana_area = 0.0;

/* Algorithm from https://www.geeksforgeeks.org/check-whether-given-point-lies-inside-rectangle-not/. */

/*
 * Helper function to determine if a point is within a rectangle.
 */
bool point_within_rectangle(location_t *point, track_elem_t *rect_tile, double rect_area) {
  double a1 = area_triangle(point, &rect_tile->upper_left, &rect_tile->upper_right);
  double a2 = area_triangle(point, &rect_tile->upper_right, &rect_tile->bottom_right);
  double a3 = area_triangle(point, &rect_tile->bottom_right, &rect_tile->bottom_left);
  double a4 = area_triangle(point, &rect_tile->bottom_left, &rect_tile->upper_left);
  return fabs(rect_area - (a1 + a2 + a3 + a4)) <= 100.0;
}

/*
 * Helper function to determine the area of a rectangle.
 */
double area_rectangle(track_elem_t *rect_tile) {
  double x_length = get_euclidean_distance(&rect_tile->upper_left, &rect_tile->upper_right);
  double y_length = get_euclidean_distance(&rect_tile->upper_left, &rect_tile->bottom_left);
  return x_length * y_length;
}

/*
 * Helper function for the area of a triangle.
 *
 * Algorithm from https://math.stackexchange.com/questions/128991/how-to-calculate-the-area-of-a-3d-triangle
 */

double area_triangle(location_t *a, location_t *b, location_t *c) {
  double x_ab = a->x - b->x;
  double x_ac = a->x - c->x;
  double y_ab = a->y - b->y;
  double y_ac = a->y - c->y;
  double z_ab = a->z - b->z;
  double z_ac = a->z - c->z;
  double s1 = y_ab * z_ac - z_ab * y_ac;
  double s2 = z_ab * x_ac - x_ab * z_ac;
  double s3 = x_ab * y_ac - y_ab * x_ac;
  double area = 0.5 * sqrt(pow(s1, 2.0) + pow(s2, 2.0) + pow(s3, 2.0));
  return area;
}


/*
 * Helper function to calculate the euclidean distance between two locations.
 */
double get_euclidean_distance(location_t *l1, location_t *l2) {
    double result = sqrt(pow(l1->x - l2->x, 2.0) + pow(l1->y - l2->y, 2.0) + pow(l1->z - l2->z, 2.0));
    return result;
}
