#include "powerups.h"
#include "../led/pwm.h"

#define POWERUP_VELOCITY_MAX 1666.0

#define MUSHROOM_DECAY_ACC -400

#define MUSHROOM_TICKS 40

uint16_t powerup_counter = 0;

/*
	Function used to apply a mushroom powerup. This increases a user's current
	and allows them to reach a higher max velocity. The powerup should then
	decay to the previous max.
*/
void apply_mushroom(power_fsm *fsm) {
	fsm->state = MUSHROOM;
  	fsm->p = POWERUP_VELOCITY_MAX;
  	fsm->p_dot = 0.0;
  	fsm->p_max = POWERUP_VELOCITY_MAX;
  	fsm->t_prev = fsm->t_curr;
  	fsm->t_curr = read_timer();
  	powerup_counter = MUSHROOM_TICKS;
  	light_red();
}

void decay_mushroom(power_fsm *fsm) {
	fsm->state = MUSHROOM_DECAY;
	fsm->p_dot = MUSHROOM_DECAY_ACC;
	fsm->t_prev = fsm->t_curr;
  	fsm->t_curr = read_timer();
	if (fsm->p <= BASE_VELOCITY_MAX) {
		complete_powerup(fsm);
	} else {
		p_update();
	}
}

void complete_powerup(power_fsm *fsm) {
	fsm->state = EXIT_POWERUP;
	fsm->p_max = BASE_VELOCITY_MAX;
	clear_lights();
	p_update();
}