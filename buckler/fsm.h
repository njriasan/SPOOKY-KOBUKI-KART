#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include "states.h"

typedef struct {
	power_states state;
	double p;
	double p_dot;
	double p_max;
	uint32_t t_curr;
	uint32_t t_prev;
} power_fsm;

typedef struct {
	turning_states state;
	double p_left;
	double p_right;
} turning_fsm;

void init_power_fsm(power_fsm *fsm);
void init_turning_fsm(turning_fsm *fsm);

void timer_init();

void rest();
void on_X_press();
void on_button_release();
void on_B_press();
void p_update();
void on_l_stick_press();
void on_r_stick_press();
void on_stick_release();
void drive();

extern power_fsm p_fsm;
extern turning_fsm t_fsm;

#endif
