#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_pwm.h"

void pwm_init(nrf_drv_pwm_t* m_pwm0_ptr);
void light_green(nrf_drv_pwm_t* m_pwm0_ptr);
void light_blue(nrf_drv_pwm_t* m_pwm0_ptr);
void light_red(nrf_drv_pwm_t* m_pwm0_ptr);


#endif