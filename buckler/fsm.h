#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include "states.h"

#define BASE_VELOCITY_MAX 1300.0


typedef struct {
	velocity_states state;
	double v;
	double v_dot;
	double v_max;
	uint32_t t_curr;
	uint32_t t_prev;
} velocity_fsm;

typedef struct {
	turning_states state;
	double v_left;
	double v_right;
} turning_fsm;

void init_velocity_fsm(velocity_fsm *fsm);
void init_turning_fsm(turning_fsm *fsm);

void timer_init();
uint32_t read_timer();

void rest();
void on_X_press();
void on_B_press();
void on_button_release();
void v_update();
void on_l_stick_press();
void on_l_up_stick_press();
void on_r_stick_press();
void on_r_up_stick_press();
void on_stick_release();
void drive();

extern velocity_fsm v_fsm;
extern turning_fsm t_fsm;

#endif
