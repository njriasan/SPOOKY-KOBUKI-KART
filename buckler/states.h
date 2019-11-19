#ifndef STATES_H
#define STATES_H

#include <stdio.h>

typedef enum {
	REST,
	ACCELERATE,
	DECELERATE,
	BRAKE
} power_states;

typedef enum {
	CENTER,
	LEFT,
	LEFT_UP,
	RIGHT,
	RIGHT_UP
} turning_states;

#endif /* STATES_H_ */
