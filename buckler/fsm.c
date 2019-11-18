#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"

#include "fsm.h"
#include "time.h"
#include "limits.h"

#define TURNING_RATE .6

power_fsm p_fsm;
turning_fsm t_fsm;

void init_power_fsm(power_fsm *fsm) {
	fsm->state = REST;
	fsm->p = 0.0f;
	fsm->p_dot = 0.0f;
	fsm->p_max = 700.0f;
	fsm->t_prev = clock();
	fsm->t_curr = fsm->t_prev;
}

void init_turning_fsm(turning_fsm *fsm) {
	fsm->state = CENTER;
	fsm->p_left = 0.0f;
	fsm->p_right = 0.0f;
}

void rest() {
	p_fsm.state = REST;
	p_fsm.t_prev = clock();
	p_fsm.t_curr = p_fsm.t_prev;

	p_update();
}

void on_X_press() {
	p_fsm.state = ACCELERATE;
	p_fsm.p_dot = 25;
	p_fsm.t_prev = p_fsm.t_curr;
	p_fsm.t_curr = clock();

	p_update();
}

void on_X_release() {
	p_fsm.state = DECELERATE;
	p_fsm.p_dot = -10;
	p_fsm.t_prev = p_fsm.t_curr;
	p_fsm.t_curr = clock();

	p_update();
}

void p_update() {

	double diff = ((double) (p_fsm.t_curr - p_fsm.t_prev)) / CLOCKS_PER_SEC;

	// Handles overflow
	// if ((p_fsm.t_curr - p_fsm.t_prev) < 0) {
	// 	diff = UINT_MAX - diff;
	// } 

	p_fsm.p = p_fsm.p + diff * p_fsm.p_dot;

	if (p_fsm.p > p_fsm.p_max) {
		p_fsm.p = p_fsm.p_max;
	} else if (p_fsm.p < 0) {
		// TODO: Implement moving backwards
		p_fsm.p = 0;
		p_fsm.state = REST;
	}
}

void on_l_stick_press() {
	t_fsm.state = LEFT;
	t_fsm.p_right = p_fsm.p * TURNING_RATE;
	t_fsm.p_left = p_fsm.p * (1 - TURNING_RATE);
}

void on_r_stick_press() {
	t_fsm.state = RIGHT;
	t_fsm.p_left = p_fsm.p * TURNING_RATE;
	t_fsm.p_right = p_fsm.p * (1 - TURNING_RATE);
}

void on_stick_release() {
	t_fsm.state = CENTER;
	t_fsm.p_left = p_fsm.p / 2.0;
	t_fsm.p_right = p_fsm.p / 2.0;
}

void drive() {
	kobukiDriveDirect(t_fsm.p_left, t_fsm.p_right);
}


