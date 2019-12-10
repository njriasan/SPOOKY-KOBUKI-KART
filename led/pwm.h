#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_pwm.h"

void pwm_init();
void light_green(uint32_t num_leds);
void light_blue();
void light_red();
void clear_lights(uint32_t num_leds);

#endif