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

#define PWM_PIN 23

static void pwm_lightup(nrf_pwm_values_common_t *arr, size_t arr_len);

nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static volatile nrf_pwm_values_common_t clear_values[24] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
};

static volatile nrf_pwm_values_common_t red_values[24] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    6, 6, 6, 6, 6, 6, 6, 6, 
    13, 13, 13, 13, 13, 13, 13, 13,
};

static volatile nrf_pwm_values_common_t green_values[24] = {
    6, 6, 6, 6, 6, 6, 6, 6, 
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
};

static volatile nrf_pwm_values_common_t blue_values[24] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    6, 6, 6, 6, 6, 6, 6, 6, 
};

static volatile nrf_pwm_values_common_t yellow_values[24] = {
    6, 6, 6, 6, 6, 6, 6, 6, 
    6, 6, 6, 6, 6, 6, 6, 6, 
    13, 13, 13, 13, 13, 13, 13, 13,
};

void pwm_init() {

    uint32_t err_code;
    // NRF_CLOCK->TASKS_HFCLKSTART = 1;
    // while(NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
    
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
        .top_value    = 20, // pwm_period = (1 / base_clock) * top_value
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    // int16_t buf[] = {(1 << 15) | 1500}; // Inverse polarity (bit 15), 1500us duty cycle
    // NRF_PWM0->SEQ[0].PTR = (uint32_t)&buf[0];
    /* Initialize the pwm instance */
    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("%s\n", "Failed inializing pwm instance");
    }
}

static void pwm_lightup(nrf_pwm_values_common_t *arr, size_t arr_len) {
    /* Define duty cycle sequence*/
    nrf_pwm_sequence_t const seq = {
        .values.p_common = arr,
        .length          = arr_len,
        .repeats         = 0,
        .end_delay       = 0
    };

    /* Play the PWM sequence on an intialized pwm instance*/
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP); // loop playback
    // nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, 0); // play once each led
}

void lightup_led(uint32_t num_leds, uint32_t color_id) {
    nrf_pwm_values_common_t *color_values;
    if (color_id == 1) {
        color_values = red_values;
    } else if (color_id == 2) {
        color_values = green_values;
    } else if (color_id == 3) {
        color_values = blue_values;
    } 
    else if (color_id == 4) {
        color_values = yellow_values;
    } else {
        color_values = clear_values;
    }

    nrf_pwm_values_common_t values[24 * num_leds + 40];

    for (uint32_t led_num = 0; led_num < num_leds; led_num++) {
        for (int i = 0; i < 24; i++) {
            values[led_num * 24 + i] = color_values[i];
            printf("%d\n", i);
        }
    }

    for (uint32_t i = 24 * num_leds; i < + 24 * num_leds + 40; i++) {
        values[i] = 20;
    }
    
    pwm_lightup(values, NRF_PWM_VALUES_LENGTH(values));
}