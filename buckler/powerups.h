#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdint.h>
#include "fsm.h"

// Information on powerup values
#define NO_POWERUP 0
#define MUSHROOM_POWERUP 1
#define REDSHELL_POWERUP 2
extern uint8_t powerup_value;

// Information on hazard values
# define NO_HAZARD 0
# define BANANA_HAZARD 1
# define REDSHELL_HAZARD 2
extern uint8_t hazard_value;

extern uint16_t powerup_counter;
extern uint16_t banana_counter;

/*
	Function used to apply a mushroom powerup. This increases a user's current
	and allows them to reach a higher max velocity. The powerup should then
	decay to the previous max.
*/
void apply_mushroom();
void decay_mushroom();
void complete_powerup();
void apply_banana();
void decay_banana();

#endif
