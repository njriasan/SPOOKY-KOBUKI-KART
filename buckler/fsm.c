#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"

#include "fsm.h"
#include "limits.h"
#include "nrf.h"

#define HARD_TURNING_RATE .6
#define SOFT_TURNING_RATE .55
#define ACCELERATION 400
#define BRAKING -800
#define DECELERATION -200

#define PRESCALE_VALUE 4
#define BASE_CLOCK 16000000.0

power_fsm p_fsm;
turning_fsm t_fsm;

static uint32_t read_timer();

static uint32_t read_timer() {
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

void init_power_fsm(power_fsm *fsm) {
  fsm->state = REST;
  fsm->p = 0.0;
  fsm->p_dot = 0.0;
  fsm->p_max = 800.0;
  fsm->t_prev = read_timer();
  fsm->t_curr = fsm->t_prev;
}

void init_turning_fsm(turning_fsm *fsm) {
  fsm->state = CENTER;
  fsm->p_left = 0.0;
  fsm->p_right = 0.0;
}

void rest() {
  p_fsm.state = REST;
  p_fsm.t_prev = p_fsm.t_curr;
  p_fsm.t_curr = read_timer();
  p_update();
}

void on_X_press() {
  p_fsm.state = ACCELERATE;
  p_fsm.p_dot = ACCELERATION;
  p_fsm.t_prev = p_fsm.t_curr;
  p_fsm.t_curr = read_timer();

  p_update();
}

void on_B_press() {
  p_fsm.state = BRAKE;
  p_fsm.p_dot = BRAKING;
  p_fsm.t_prev = p_fsm.t_curr;
  p_fsm.t_curr = read_timer();

  p_update();
}

void on_button_release() {
  p_fsm.state = DECELERATE;
  p_fsm.p_dot = DECELERATION;
  p_fsm.t_prev = p_fsm.t_curr;
  p_fsm.t_curr = read_timer();

  p_update();
}

void p_update() {

  double change = 0.0;
  if (p_fsm.t_curr < p_fsm.t_prev) {
    uint32_t rescaled_curr = UINT32_MAX - p_fsm.t_curr;
    change = (double) (rescaled_curr + 1 + p_fsm.t_prev);
  } else {
    change = (double) (p_fsm.t_curr - p_fsm.t_prev);
  }
  double diff = (change) / (BASE_CLOCK / (1 << PRESCALE_VALUE));
  printf("%lf\n", diff);

  p_fsm.p = p_fsm.p + diff * p_fsm.p_dot;
  printf("%lf\n\n\n\n", p_fsm.p);


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

void on_l_up_stick_press() {
  t_fsm.state = LEFT_UP;
  t_fsm.p_right = p_fsm.p * SOFT_TURNING_RATE;
  t_fsm.p_left = p_fsm.p * (1 - SOFT_TURNING_RATE);
}

void on_r_stick_press() {
  t_fsm.state = RIGHT;
  t_fsm.p_left = p_fsm.p * TURNING_RATE;
  t_fsm.p_right = p_fsm.p * (1 - TURNING_RATE);
}

void on_r_up_stick_press() {
  t_fsm.state = RIGHT_UP;
  t_fsm.p_left = p_fsm.p * SOFT_TURNING_RATE;
  t_fsm.p_right = p_fsm.p * (1 - SOFT_TURNING_RATE);
}

void on_stick_release() {
  t_fsm.state = CENTER;
  t_fsm.p_left = p_fsm.p / 2.0;
  t_fsm.p_right = p_fsm.p / 2.0;
}

void drive() {
  kobukiDriveDirect(t_fsm.p_left, t_fsm.p_right);
}


