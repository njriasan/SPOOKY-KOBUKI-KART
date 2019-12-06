#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"

int main(void) {
    /* Define PWM duty cycle sequence */
	pwm_init();
	lightup_led(5, 1);
	nrf_delay_ms(1000);
	lightup_led(5, 0);
	nrf_delay_ms(1000);
	lightup_led(5, 4);
	while (1) {
		// lightup_led(5, 1);
		// nrf_delay_ms(1000);
		// lightup_led(5, 2);
		// nrf_delay_ms(1000);
		// lightup_led(5, 3);
		// nrf_delay_ms(1000);
		// lightup_led(5, 0);
		// nrf_delay_ms(1000);
	}
}