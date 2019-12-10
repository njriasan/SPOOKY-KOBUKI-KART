#include "powerups.h"
#include "../led/pwm.h"

#include "nrf_delay.h"

#define POWERUP_VELOCITY_MAX 1666.0

#define MUSHROOM_DECAY_ACC -400

#define MUSHROOM_TICKS 100

#define BANANA_TICKS 200

#define BANANA_TURN_VELOCITY 400

uint8_t powerup_byte = 0;
uint8_t hazard_byte = 0;


uint16_t powerup_counter = 0;
uint16_t banana_counter = 0;

/*
	Function used to apply a mushroom powerup. This increases a user's current
	and allows them to reach a higher max velocity. The powerup should then
	decay to the previous max.
*/
void apply_mushroom() {
    v_fsm.state = MUSHROOM;
  	v_fsm.v = POWERUP_VELOCITY_MAX;
  	v_fsm.v_dot = 0.0;
  	v_fsm.v_max = POWERUP_VELOCITY_MAX;
  	v_fsm.t_prev = v_fsm.t_curr;
  	v_fsm.t_curr = read_timer();
  	powerup_counter = MUSHROOM_TICKS;
  	lightup_led(5, 2);
  	nrf_delay_ms(1);
}

void decay_mushroom() {
	powerup_byte = 0;
	v_fsm.state = MUSHROOM_DECAY;
	v_fsm.v_dot = MUSHROOM_DECAY_ACC;
	v_fsm.t_prev = v_fsm.t_curr;
  	v_fsm.t_curr = read_timer();
	if (v_fsm.v <= BASE_VELOCITY_MAX) {
		complete_powerup();
	} else {
		v_update();
	}
}

void apply_banana() {
	printf("%s\n", "here in banana function");
	complete_powerup();
	t_fsm.state = BANANA;
	t_fsm.v_left = BANANA_TURN_VELOCITY;
	t_fsm.v_right = 0.0;
	banana_counter = BANANA_TICKS;
	lightup_led(5, 3);
	nrf_delay_ms(1);
}

void decay_banana() {
	t_fsm.state = CENTER;
	hazard_byte = 0;
}

void complete_powerup() {
	powerup_counter = 0;
	v_fsm.state = EXIT_POWERUP;
	v_fsm.v_max = BASE_VELOCITY_MAX;
	powerup_byte = 0;
	//clear_lights();
	v_update();
}
