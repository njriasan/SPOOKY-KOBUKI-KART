#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "track_events.h"

location_t mushroom_tile = {1746.000000, 10465.000000, 753.000000};

location_t redshell_tile = {2500.000000, 10015.000000, 50.000000};

location_t banana_tile = {3816.000000, 11690.000000, 515.000000};



/*
 * Helper function to calculate the euclidean distance between two locations.
 */
double get_euclidean_distance(location_t* l1, location_t* l2) {
  double result = sqrt(pow(l1->x - l2->x, 2.0) + pow(l1->y - l2->y, 2.0) + pow(l1->z - l2->z, 2.0));
  // printf("Result is %lf\n", result);
  return result;
}

