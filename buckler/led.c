#include "led.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_drv_pwm.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"

#include <stdint.h>
#include <stdio.h>

#include "nrf_drv_pwm.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"

#define PWM_PIN 4
#define NUM_LEDS 6
#define zero_bit 6UL | (0x8000)
#define one_bit 13UL | (0x8000)
#define reset_bit 0UL | (0x8000)

nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static volatile nrf_pwm_values_common_t clear_values[NUM_LEDS * 24];

static volatile nrf_pwm_values_common_t red_values[NUM_LEDS * 24];

static volatile nrf_pwm_values_common_t blue_values[NUM_LEDS * 24];

static volatile nrf_pwm_values_common_t green_values[NUM_LEDS * 24];

static volatile nrf_pwm_values_common_t yellow_values[NUM_LEDS * 24];

static volatile nrf_pwm_values_common_t reset_values[40];

static void pwm_init();
static void pwm_start(nrf_pwm_values_common_t* arr, size_t arr_len);

static void pwm_init() {

}

static void pwm_start(nrf_pwm_values_common_t* pwm_values, size_t pwm_values_len) {
}

void led_init() {
}

void lightup_led(uint32_t color_id) {
}

