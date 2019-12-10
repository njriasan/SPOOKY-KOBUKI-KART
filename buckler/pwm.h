#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_pwm.h"

void pwm_init();
void lightup_led(uint32_t num_leds, uint32_t color_id);

#endif