#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_pwm.h"

void led_init();
void lightup_led(uint32_t color_id);

#endif

