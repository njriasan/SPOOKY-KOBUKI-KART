#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"


#define NUM_LEDS 7

int main(void) {
    /* Define PWM duty cycle sequence */
	pwm_init();
	while (1) {
		light_green();
		nrf_delay_ms(1000);
		light_blue();
		nrf_delay_ms(1000);
		light_red();
		nrf_delay_ms(1000);
		// clear_lights0);
	}
}