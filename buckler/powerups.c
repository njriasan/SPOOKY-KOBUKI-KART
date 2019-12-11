#include "powerups.h"
#include "../led/pwm.h"

#include "nrf_delay.h"

#define POWERUP_VELOCITY_MAX 1666.0

#define MUSHROOM_DECAY_ACC -400

#define MUSHROOM_TICKS 3.0

#define HAZARD_TICKS 5.0

#define HAZARD_TURN_VELOCITY 400

// Value of the owned powerup/hazard
uint8_t powerup_value = NO_POWERUP;
uint8_t hazard_value = NO_HAZARD;

// Whether or not the powerup/hazard is activated
bool active_powerup = false;
bool active_hazard = false;


double powerup_duration = 0.0;
double hazard_duration = 0.0;
uint32_t powerup_starttime = 0;
uint32_t hazard_starttime = 0;
uint32_t compare_time = 0;

/*
	Function used to apply a mushroom powerup. This increases a user's current
	and allows them to reach a higher max velocity. The powerup should then
	decay to the previous max.
*/
void apply_mushroom() {
    active_powerup = true;
    v_fsm.state = MUSHROOM;
  	v_fsm.v = POWERUP_VELOCITY_MAX;
  	v_fsm.v_dot = 0.0;
  	v_fsm.v_max = POWERUP_VELOCITY_MAX;
  	v_fsm.t_prev = v_fsm.t_curr;
  	v_fsm.t_curr = read_timer();
  	powerup_duration = MUSHROOM_TICKS;
    powerup_starttime = read_timer();
    // Add light logic here
    printf("%s\n", "mushroom applied");
}

void apply_redshell_powerup() {
    shell_byte = REDSHELL_BYTE;
    APP_ERROR_CHECK(simple_ble_notify_char(&shell_char));
    complete_powerup();
}

void apply_blueshell_powerup() {
    shell_byte = BLUESHELL_BYTE;
    APP_ERROR_CHECK(simple_ble_notify_char(&shell_char));
    complete_powerup();
}

void decay_mushroom() {
    active_powerup = false;
	v_fsm.state = MUSHROOM_DECAY;
	v_fsm.v_dot = MUSHROOM_DECAY_ACC;
	v_fsm.t_prev = v_fsm.t_curr;
  	v_fsm.t_curr = read_timer();
	if (v_fsm.v <= BASE_VELOCITY_MAX) {
		complete_powerup();
	} else {
		v_update();
	}
	powerup_value = NO_POWERUP;
}

void apply_banana() {
    active_hazard = true;
	complete_powerup();
	t_fsm.state = BANANA;
	t_fsm.v_left = HAZARD_TURN_VELOCITY;
	t_fsm.v_right = 0.0;
    hazard_starttime = read_timer();
	hazard_duration = HAZARD_TICKS;
}

void apply_redshell_hazard() {
    active_hazard = true;
    complete_powerup();
	t_fsm.state = REDSHELL;
	t_fsm.v_left = HAZARD_TURN_VELOCITY;
	t_fsm.v_right = 0.0;
	hazard_duration = HAZARD_TICKS;
    // Add lights information
}

void apply_blueshell_hazard() {
    active_hazard = true;
    complete_powerup();
	t_fsm.state = BLUESHELL;
	t_fsm.v_left = HAZARD_TURN_VELOCITY;
	t_fsm.v_right = 0.0;
	hazard_duration = HAZARD_TICKS;
    // Add lights information
}

void decay_hazard() {
    active_hazard = false;
	t_fsm.state = CENTER;
	hazard_value = NO_HAZARD;
	hazard_duration = 0;
}

void complete_powerup() {
    active_powerup = false;
	powerup_duration = 0;
	v_fsm.state = EXIT_POWERUP;
	v_fsm.v_max = BASE_VELOCITY_MAX;
	powerup_value = NO_POWERUP;
	//clear_lights();
	v_update();
}
