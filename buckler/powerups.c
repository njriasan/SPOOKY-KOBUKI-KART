#include "powerups.h"
#include "led.h"

#include "nrf_delay.h"

#define POWERUP_VELOCITY_MAX 1666.0

#define MUSHROOM_DECAY_ACC -400

#define MUSHROOM_TICKS 3.0

#define HAZARD_TICKS 5.0

#define HAZARD_TURN_VELOCITY 400

// Value of the owned powerup/hazard
uint8_t powerup_value = NO_POWERUP;
uint8_t hazard_value  = NO_HAZARD;

// Backup value for rz button
uint8_t rz_backup = 0;

// Whether or not the powerup/hazard is activated
bool active_powerup = false;
bool active_hazard  = false;

// Determines if the shell characteristic needs to be notified.
// This is set to true if the call to simple_ble_notify_char fails.
bool shell_not_notified = false;

double powerup_duration    = 0.0;
double hazard_duration     = 0.0;
uint32_t powerup_starttime = 0;
uint32_t hazard_starttime  = 0;
uint32_t compare_time      = 0;

/*
 *  Function used to apply a mushroom powerup. This increases a user's current
 *  and allows them to reach a higher max velocity. The powerup should then
 *  decay to the previous max.
 */
void apply_mushroom() {
  printf("%s\n", "enters mushroom function");
  active_powerup    = true;
  v_fsm.state       = MUSHROOM;
  v_fsm.v           = POWERUP_VELOCITY_MAX;
  v_fsm.v_dot       = 0.0;
  v_fsm.v_max       = POWERUP_VELOCITY_MAX;
  v_fsm.t_prev      = v_fsm.t_curr;
  v_fsm.t_curr      = read_timer();
  powerup_duration  = MUSHROOM_TICKS;
  powerup_starttime = read_timer();
  lightup_led(0);
  powerup_value = NO_POWERUP;
}

void apply_redshell_powerup() {
  printf("%s\n", "enters red shell function");
  shell_byte = REDSHELL_BYTE;
  uint32_t err_code = simple_ble_notify_char(&shell_char);
  if (err_code != NRF_SUCCESS) {
    shell_not_notified = true;
  }
  complete_powerup();
  lightup_led(0);
  powerup_value = NO_POWERUP;
}

void apply_blueshell_powerup() {
  shell_byte = BLUESHELL_BYTE;
  uint32_t err_code = simple_ble_notify_char(&shell_char);
  if (err_code != NRF_SUCCESS) {
    shell_not_notified = true;
  }
  complete_powerup();
  lightup_led(0);
  powerup_value = NO_POWERUP;
}

void decay_mushroom() {
  active_powerup = false;
  v_fsm.state    = MUSHROOM_DECAY;
  v_fsm.v_dot    = MUSHROOM_DECAY_ACC;
  v_fsm.t_prev   = v_fsm.t_curr;
  v_fsm.t_curr   = read_timer();
  if (v_fsm.v <= BASE_VELOCITY_MAX) {
    complete_powerup();
  } else {
    v_update();
  }
}

void apply_banana() {
  active_hazard = true;
  complete_powerup();
  powerup_value    = NO_POWERUP;
  v_fsm.state      = REST;
  v_fsm.v          = 0.0;
  v_fsm.v_dot      = 0.0;
  t_fsm.state      = BANANA;
  t_fsm.v_left     = HAZARD_TURN_VELOCITY;
  t_fsm.v_right    = 0.0;
  hazard_starttime = read_timer();
  hazard_duration  = HAZARD_TICKS;
}

void apply_redshell_hazard() {
  printf("%s\n", "redshell hazard triggered");
  active_hazard = true;
  complete_powerup();
  powerup_value    = NO_POWERUP;
  v_fsm.state      = REST;
  v_fsm.v          = 0.0;
  v_fsm.v_dot      = 0.0;
  t_fsm.state      = REDSHELL;
  t_fsm.v_left     = HAZARD_TURN_VELOCITY;
  t_fsm.v_right    = 0.0;
  hazard_starttime = read_timer();
  hazard_duration  = HAZARD_TICKS;
}

void apply_blueshell_hazard() {
  active_hazard = true;
  complete_powerup();
  powerup_value    = NO_POWERUP;
  v_fsm.state      = REST;
  v_fsm.v          = 0.0;
  v_fsm.v_dot      = 0.0;
  t_fsm.state      = BLUESHELL;
  t_fsm.v_left     = HAZARD_TURN_VELOCITY;
  t_fsm.v_right    = 0.0;
  hazard_starttime = read_timer();
  hazard_duration  = HAZARD_TICKS;
}

void decay_hazard() {
  active_hazard = false;
  t_fsm.state   = CENTER;
  lightup_led(0);
  hazard_value    = NO_HAZARD;
  hazard_duration = 0;
}

void complete_powerup() {
  active_powerup   = false;
  powerup_duration = 0;
  rz_button.value = rz_backup;
  v_fsm.state      = EXIT_POWERUP;
  v_fsm.v_max      = BASE_VELOCITY_MAX;
  v_fsm.v_dot      = ACCELERATION;
  v_update();
}

