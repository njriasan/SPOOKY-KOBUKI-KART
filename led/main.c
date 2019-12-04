#include <stdint.h>
#include <stdio.h>

#include "nrf_drv_pwm.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"


#define PWM_PIN 26

/* Define a pwm instance */
static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static void pwm_init() {
    uint32_t err_code;

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
}

void pwm_start(uint32_t num_leds) {

    /* Define PWM duty cycle sequence */
    static nrf_pwm_values_common_t seq_values[] = {
        6, 6, 6, 6, 6, 6, 6, 6,
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
        100, 100
    };

    /* Define duty cycle sequence*/
    nrf_pwm_sequence_t const seq = {
        .values.p_individual = seq_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats         = 0,
        .end_delay       = 0
    };

    /* Play the PWM sequence on an intialized pwm instance*/
    // nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP); // loop playback
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, num_leds, 0); // play once each led
}

int main(void) {
    uint32_t num_leds = 7;
    pwm_init();
    pwm_start(num_leds);
}
