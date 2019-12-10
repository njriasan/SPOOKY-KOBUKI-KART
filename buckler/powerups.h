#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdint.h>
#include "fsm.h"
#include "simple_ble.h"

// Information on powerup values
#define NO_POWERUP 0
#define MUSHROOM_POWERUP 1
#define REDSHELL_POWERUP 2
#define BLUESHELL_POWERUP 3
extern uint8_t powerup_value;

// Information on hazard values
# define NO_HAZARD 0
# define BANANA_HAZARD 1
# define REDSHELL_HAZARD 2
# define BLUESHELL_HAZARD 3
extern uint8_t hazard_value;

extern uint16_t powerup_counter;
extern uint16_t banana_counter;

// Information on the shell sending BLE char
#define NO_SHELL_BYTE 0
#define REDSHELL_BYTE 1
#define BLUESHELL_BYTE 1
extern simple_ble_char_t shell_char;
extern uint8_t shell_byte;


/*
	Function used to apply a mushroom powerup. This increases a user's current
	and allows them to reach a higher max velocity. The powerup should then
	decay to the previous max.
*/
void apply_mushroom();
void apply_redshell_powerup();
void apply_blueshell_powerup();
void decay_mushroom();
void complete_powerup();
void apply_banana();
void apply_redshell_hazard();
void apply_blueshell_hazard();
void decay_banana();
void decay_redshell_hazard();
void decay_blueshell_hazard();

#endif
