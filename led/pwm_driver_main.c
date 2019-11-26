
#include <stdio.h>
#include <string.h>
#include "nrf_drv_pwm.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "boards.h"
#include "bsp.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"


#define PWM_PIN 27

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

// Declare variables holding PWM sequence values. In this example only one channel is used 
nrf_pwm_values_individual_t seq_values[] = {0, 0, 0, 0};

nrf_pwm_sequence_t const seq = {
    .values.p_individual = seq_values,
    .length          = NRF_PWM_VALUES_LENGTH(seq_values),
    .repeats         = 0,
    .end_delay       = 0
};

// Set duty cycle between 0 and 100%
void pwm_update_duty_cycle(uint8_t duty_cycle) {
    
    // Check if value is outside of range. If so, set to 100%
    if (duty_cycle >= 100) {
        seq_values->channel_0 = 100;
    }
    else {
        seq_values->channel_0 = duty_cycle;
    }
    
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}

static void pwm_init() {
    uint32_t err_code;
    nrf_drv_pwm_config_t const config0 = {
        .output_pins =
        {
            PWM_PIN,                              // channel 0
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 1
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 2
            NRF_DRV_PWM_PIN_NOT_USED,             // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 100,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    if (err_code != NRF_SUCCESS) {
        // Initialization failed. Take recovery action.
    }
}


int main(void) {

    // Start clock for accurate frequencies
    NRF_CLOCK->TASKS_HFCLKSTART = 1; 
    // Wait for clock to start
    while(NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
    
    pwm_init();

    // for (;;) {
    //     for(int i = 0; i <= 100; i++) {
    //         nrf_delay_ms(10);
    //         pwm_update_duty_cycle(i);
    //     }
    // }
    pwm_update_duty_cycle(100);
}
