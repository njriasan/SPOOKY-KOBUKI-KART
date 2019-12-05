#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"

#include "fsm.h"
#include "limits.h"
#include "nrf.h"

#define HARD_TURNING_RATE .6
#define SOFT_TURNING_RATE .55
#define ACCELERATION 800
#define BRAKING -1400

#define PRESCALE_VALUE 4
#define BASE_CLOCK 16000000.0

velocity_fsm v_fsm;
turning_fsm t_fsm;


uint32_t read_timer() {
  NRF_TIMER4->TASKS_CAPTURE[1] = 0x1;
  return NRF_TIMER4->CC[1];
}

void timer_init(void) {
  // Place your timer initialization code here
  NRF_TIMER4->BITMODE = 0x3;
  NRF_TIMER4->PRESCALER = PRESCALE_VALUE;
  NRF_TIMER4->INTENSET = 0;
  NRF_TIMER4->TASKS_CLEAR = 0x1;
  NRF_TIMER4->TASKS_START = 0x1;
}

void init_velocity_fsm(velocity_fsm *fsm) {
  fsm->state = REST;
  fsm->v = 0.0;
  fsm->v_dot = 0.0;
  fsm->v_max = BASE_VELOCITY_MAX;
  fsm->t_prev = read_timer();
  fsm->t_curr = fsm->t_prev;
}

void init_turning_fsm(turning_fsm *fsm) {
  fsm->state = CENTER;
  fsm->v_left = 0.0;
  fsm->v_right = 0.0;
}

void rest() {
  v_fsm.state = REST;
  v_fsm.t_prev = v_fsm.t_curr;
  v_fsm.t_curr = read_timer();
  v_update();
}

void on_X_press() {
  v_fsm.state = ACCELERATE;

  if (v_fsm.v < 0) {
    v_fsm.v_dot = -1 * BRAKING;
  } else {
    v_fsm.v_dot = ACCELERATION;
  }

  v_fsm.t_prev = v_fsm.t_curr;
  v_fsm.t_curr = read_timer();

  v_update();
}

void on_B_press() {
  v_fsm.state = REVERSE;

  if (v_fsm.v > 0) {
    v_fsm.v_dot = BRAKING;
  } else {
    v_fsm.v_dot = -1 * ACCELERATION;
  }

  v_fsm.t_prev = v_fsm.t_curr;
  v_fsm.t_curr = read_timer();

  v_update();
}

void on_button_release() {
  if (v_fsm.v > 0) {
    v_fsm.v_dot = -ACCELERATION / 2;
  } else {
    v_fsm.v_dot = ACCELERATION / 2;
  }
  v_fsm.state = CRUISE;
  v_fsm.t_prev = v_fsm.t_curr;
  v_fsm.t_curr = read_timer();

  v_update();
}

void v_update() {

  double change = 0.0;
  if (v_fsm.t_curr < v_fsm.t_prev) {
    uint32_t rescaled_curr = UINT32_MAX - v_fsm.t_curr;
    change = (double) (rescaled_curr + 1 + v_fsm.t_prev);
  } else {
    change = (double) (v_fsm.t_curr - v_fsm.t_prev);
  }
  double diff = (change) / (BASE_CLOCK / (1 << PRESCALE_VALUE));

  v_fsm.v = v_fsm.v + diff * v_fsm.v_dot;


  if (v_fsm.v >= v_fsm.v_max) {
    v_fsm.v = v_fsm.v_max;
  } else if (v_fsm.v <= -v_fsm.v_max) {
    v_fsm.v = -v_fsm.v_max;
  } else if ((v_fsm.v < 10 && v_fsm.v > -10) && v_fsm.state == CRUISE) {
    v_fsm.v_dot = 0;
    v_fsm.v = 0;
    v_fsm.state = REST;
  }
}

void on_l_stick_press() {
  t_fsm.state = LEFT;
  t_fsm.v_right = v_fsm.v * HARD_TURNING_RATE;
  t_fsm.v_left = v_fsm.v * (1 - HARD_TURNING_RATE);
}

void on_l_up_stick_press() {
  t_fsm.state = LEFT_UP;
  t_fsm.v_right = v_fsm.v * SOFT_TURNING_RATE;
  t_fsm.v_left = v_fsm.v * (1 - SOFT_TURNING_RATE);
}

void on_r_stick_press() {
  t_fsm.state = RIGHT;
  t_fsm.v_left = v_fsm.v * HARD_TURNING_RATE;
  t_fsm.v_right = v_fsm.v * (1 - HARD_TURNING_RATE);
}

void on_r_up_stick_press() {
  t_fsm.state = RIGHT_UP;
  t_fsm.v_left = v_fsm.v * SOFT_TURNING_RATE;
  t_fsm.v_right = v_fsm.v * (1 - SOFT_TURNING_RATE);
}

void on_stick_release() {
  t_fsm.state = CENTER;
  t_fsm.v_left = v_fsm.v / 2.0;
  t_fsm.v_right = v_fsm.v / 2.0;
}

void drive() {
  kobukiDriveDirect(t_fsm.v_left, t_fsm.v_right);
}


