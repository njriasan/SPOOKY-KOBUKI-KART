#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_pwm.h"

void pwm_init();
void light_green();
void light_blue();
void light_red();
void clear_lights();

#endif