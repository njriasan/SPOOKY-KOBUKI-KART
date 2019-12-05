#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"


#define NUM_LEDS 7

int main(void) {
    /* Define PWM duty cycle sequence */
    nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);
	pwm_init(&m_pwm0);
	while (1) {
		light_green(&m_pwm0);
		nrf_delay_ms(1000);
		light_blue(&m_pwm0);
		nrf_delay_ms(1000);
		light_red(&m_pwm0);
		nrf_delay_ms(1000);
	}
}
