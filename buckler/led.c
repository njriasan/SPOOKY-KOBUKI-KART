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

#define PWM_PIN 3
#define NUM_LEDS 7
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
static void pwm_start(nrf_pwm_values_common_t *arr, size_t arr_len);

static void pwm_init() {

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

    
    /* Initialize the pwm instance */
    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    if (err_code != NRF_SUCCESS) {
        printf("%s\n", "Failed inializing pwm instance");
    }

    for (uint32_t i = 0; i < 40; i++) {
        reset_values[i] = reset_bit;
    }

    for (uint32_t led_num = 0; led_num < NUM_LEDS; led_num++) {
        // fill clear_values with pwm duty cycle values
        for (uint32_t i = 0; i < 24; i++) {
            clear_values[led_num * 24 + i] = zero_bit;
        }

        // fill red_values with pwm duty cycle values
        for (uint32_t i = 0; i < 8; i++) {
            red_values[led_num * 24 + i] = zero_bit;
        }
        for (uint32_t i = 8; i < 16; i++) {
            red_values[led_num * 24 + i] = one_bit;
        }
        for (uint32_t i = 16; i < 24; i++) {
            red_values[led_num * 24 + i] = zero_bit;
        }

        // fill green_values with pwm duty cycle values
        for (uint32_t i = 0; i < 8; i++) {
            green_values[led_num * 24 + i] = one_bit;
        }
        for (uint32_t i = 8; i < 16; i++) {
            green_values[led_num * 24 + i] = zero_bit;
        }
        for (uint32_t i = 16; i < 24; i++) {
            green_values[led_num * 24 + i] = zero_bit;
        }

        // fill blue_values with pwm duty cycle values
        for (uint32_t i = 0; i < 8; i++) {
            blue_values[led_num * 24 + i] = zero_bit;
        }
        for (uint32_t i = 8; i < 16; i++) {
            blue_values[led_num * 24 + i] = zero_bit;
        }
        for (uint32_t i = 16; i < 24; i++) {
            blue_values[led_num * 24 + i] = one_bit;
        }

        // fill yellow_values with pwm duty cycle values
        for (uint32_t i = 0; i < 8; i++) {
            yellow_values[led_num * 24 + i] = one_bit;
        }
        for (uint32_t i = 8; i < 16; i++) {
            yellow_values[led_num * 24 + i] = one_bit;
        }
        for (uint32_t i = 16; i < 24; i++) {
            yellow_values[led_num * 24 + i] = zero_bit;
        }
    }
}

static void pwm_start(nrf_pwm_values_common_t *pwm_values, size_t pwm_values_len) {
    /* Define duty cycle sequence*/
    nrf_pwm_sequence_t const seq = {
        .values.p_common = pwm_values,
        .length          = pwm_values_len,
        .repeats         = 0,
        .end_delay       = 0
    };

    nrf_pwm_sequence_t const reset_seq = {
        .values.p_common = reset_values,
        .length          = NRF_PWM_VALUES_LENGTH(reset_values),
        .repeats         = 0,
        .end_delay       = 0
    };

    /* Play the PWM sequence on an intialized pwm instance*/
    nrf_drv_pwm_complex_playback(&m_pwm0, &seq, &reset_seq, 1, NRF_DRV_PWM_FLAG_LOOP); // loop playback
    // nrf_drv_pwm_complex_playback(&m_pwm0, &seq, &reset_seq, 1, 0); // play once each led
}

void led_init() {
    pwm_init();
}

void lightup_led(uint32_t color_id) {
    if (color_id == 1) {
        pwm_start(&red_values, NRF_PWM_VALUES_LENGTH(red_values));
    } else if (color_id == 2) {
        pwm_start(&green_values, NRF_PWM_VALUES_LENGTH(green_values));
    } else if (color_id == 3) {
        pwm_start(&blue_values, NRF_PWM_VALUES_LENGTH(blue_values));
    } else if (color_id == 4) {
        pwm_start(&yellow_values, NRF_PWM_VALUES_LENGTH(yellow_values));
    } else {
        pwm_start(&clear_values, NRF_PWM_VALUES_LENGTH(clear_values));
    }
}
