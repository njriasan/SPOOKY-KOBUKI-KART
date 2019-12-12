#include "pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"

int main(void) {
    /* Define PWM duty cycle sequence */
	led_init();
	lightup_led(2);

	while (1) {

	}
}