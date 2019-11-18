#ifndef STATES_H
#define STATES_H

#include <stdio.h>

typedef enum {
	REST,
	ACCELERATE,
	DECELERATE
} power_states;

typedef enum {
	CENTER,
	LEFT,
	RIGHT
} turning_states;

#endif /* STATES_H_ */
