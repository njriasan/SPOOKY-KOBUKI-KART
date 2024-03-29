#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdbool.h>
#include <stdint.h>
#include "fsm.h"
#include "simple_ble.h"

/* Struct for buttons. */
typedef struct {
  const char* name;
  uint16_t mask;
  uint8_t shift_amount;
  uint8_t value;
} button_info_t;

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

extern bool active_powerup;
extern bool active_hazard;

// Fix a bug on missed powerup usage
extern button_info_t rz_button;
extern uint8_t rz_backup;

// Determines if the shell characteristic needs to be notified.
// This is set to true if the call to simple_ble_notify_char fails.
extern bool shell_not_notified;


// Timers used to calculate powerup duration
extern double powerup_duration;
extern double hazard_duration;
extern uint32_t powerup_starttime;
extern uint32_t hazard_starttime;
extern uint32_t compare_time;

// Information on the shell sending BLE char
#define NO_SHELL_BYTE 0
#define REDSHELL_BYTE 1
#define BLUESHELL_BYTE 2
extern simple_ble_char_t shell_char;
extern uint8_t shell_byte;


/*
 *  Function used to apply a mushroom powerup. This increases a user's current
 *  and allows them to reach a higher max velocity. The powerup should then
 *  decay to the previous max.
 */
void apply_mushroom();
void apply_redshell_powerup();
void apply_blueshell_powerup();
void decay_mushroom();
void complete_powerup();
void apply_banana();
void apply_redshell_hazard();
void apply_blueshell_hazard();
void decay_hazard();

#endif

