#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdint.h>
#include "fsm.h"

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