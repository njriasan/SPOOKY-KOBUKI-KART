#ifndef STATES_H
#define STATES_H

#include <stdio.h>

typedef enum {
	REST,
	ACCELERATE,
	REVERSE,
	CRUISE,
	MUSHROOM,
	MUSHROOM_DECAY,
	EXIT_POWERUP
} power_states;

typedef enum {
	CENTER,
	LEFT,
	LEFT_UP,
	LEFT_DOWN,
	RIGHT,
	RIGHT_UP,
	RIGHT_DOWN
} turning_states;

#endif /* STATES_H_ */
