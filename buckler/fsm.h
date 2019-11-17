#ifndef FSM_H
#define FSM_H

#include "time.h"
#include "states.h"

typedef struct {
	power_states state;
	float p;
	float p_dot;
	float p_max;
	clock_t t_curr;
	clock_t t_prev;
} power_fsm;

typedef struct {
	turning_states state;
	float p_left;
	float p_right;
} turning_fsm;

void rest();
void on_X_press();
void on_X_release();
void p_update();
void on_l_stick_press();
void on_r_stick_press();
void on_stick_release();
void drive();

#endif
