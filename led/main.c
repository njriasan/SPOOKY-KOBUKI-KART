#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"

#include "buckler.h"

#include "app_pwm.h" //Add pwm header file

APP_PWM_INSTANCE(PWM1,1);  // Create the instance "PWM1" using TIMER1

static volatile bool ready_flag; // A flag indicating PWM status.

// PWM callback function
void pwm_ready_callback(uint32_t pwm_id) {   
    ready_flag = true;
    printf("%s\n", "pwm in ready state");
}

int main(void) {

    /* Configure pwm channel period and pin number */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(5000.0, 27); 

    /* Switch the polarity of the second channel. */
    pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    /* Initialize and enable PWM. */
    ret_code_t err_code;
    err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback); 
    APP_ERROR_CHECK(err_code);
    
    app_pwm_enable(&PWM1); // Enable pwm
    printf("%s\n", "Done enabling pwm");

    /* Set pwm duty cycle */
    app_pwm_channel_duty_set(&PWM1, 0, 80);
    

    while (true) {

    }
}
