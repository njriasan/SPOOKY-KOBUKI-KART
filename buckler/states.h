#ifndef STATES_H_
#define STATES_H_

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
