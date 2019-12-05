#include "pwm.h"

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

#define PWM_PIN 31
#define NUM_LEDS 7

static void pwm_lightup(uint32_t num_leds, nrf_drv_pwm_t* m_pwm0_ptr, nrf_pwm_values_common_t *arr, size_t arr_len);

static volatile nrf_pwm_values_common_t red_values[26] = {
	13, 13, 13, 13, 13, 13, 13, 13,
    7, 7, 7, 7, 7, 7, 7, 7, 
    13, 13, 13, 13, 13, 13, 13, 13,
    100, 100
 };

static volatile nrf_pwm_values_common_t green_values[26] = {
    7, 7, 7, 7, 7, 7, 7, 7,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    100, 100
 };

static volatile nrf_pwm_values_common_t blue_values[26] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    7, 7, 7, 7, 7, 7, 7, 7,
    100, 100
 };

void pwm_init(nrf_drv_pwm_t* m_pwm0_ptr) {
    uint32_t err_code;

    assert(m_pwm0_ptr != NULL);

    /* Configure a pwm instance */
    nrf_drv_pwm_config_t const config0 = {
        .output_pins =
        {
            PWM_PIN,                              
            NRF_DRV_PWM_PIN_NOT_USED,             
            NRF_DRV_PWM_PIN_NOT_USED,             
            NRF_DRV_PWM_PIN_NOT_USED,             
        },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock   = NRF_PWM_CLK_16MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 21, // pwm_period = (1 / base_clock) * top_value
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    /* Initialize the pwm instance */
    err_code = nrf_drv_pwm_init(m_pwm0_ptr, &config0, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("%s\n", "Failed inializing pwm instance");
    }
}

static void pwm_lightup(uint32_t num_leds, nrf_drv_pwm_t* m_pwm0_ptr, nrf_pwm_values_common_t *arr, size_t arr_len) {

    assert(m_pwm0_ptr != NULL);
    assert(arr != NULL);

    /* Define duty cycle sequence*/
    nrf_pwm_sequence_t const seq = {
        .values.p_common = arr,
        .length          = arr_len,
        .repeats         = 0,
        .end_delay       = 0
    };

    /* Play the PWM sequence on an intialized pwm instance*/
    //nrf_drv_pwm_simple_playback(m_pwm0_ptr, &seq, 1, NRF_DRV_PWM_FLAG_LOOP); // loop playback
    nrf_drv_pwm_simple_playback(m_pwm0_ptr, &seq, num_leds, 0); // play once each led
}

void light_green(nrf_drv_pwm_t* m_pwm0_ptr) {
    pwm_lightup(NUM_LEDS, m_pwm0_ptr, green_values, NRF_PWM_VALUES_LENGTH(green_values));
}

void light_blue(nrf_drv_pwm_t* m_pwm0_ptr) {
    pwm_lightup(NUM_LEDS, m_pwm0_ptr, blue_values, NRF_PWM_VALUES_LENGTH(blue_values));
}

void light_red(nrf_drv_pwm_t* m_pwm0_ptr) {
    pwm_lightup(NUM_LEDS, m_pwm0_ptr, red_values, NRF_PWM_VALUES_LENGTH(red_values));
}
