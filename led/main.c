#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"


#define NUM_LEDS 7

int main(void) {
    /* Define PWM duty cycle sequence */
	pwm_init();
	// light_green(4);
	while (1) {
		light_green(5);
		nrf_delay_ms(1000);
		clear_lights(5);
		nrf_delay_ms(1000);

		light_green(7);
		nrf_delay_ms(1000);
		clear_lights(7);
		nrf_delay_ms(1000);
	}
}