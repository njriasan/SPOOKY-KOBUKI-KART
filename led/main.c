#include "led.h"
#include "nrf_drv_pwm.h"
#include "nrf_delay.h"

int main(void) {
  /* Define PWM duty cycle sequence */
  led_init();
  lightup_led(4);
  // nrf_delay_ms(1000);

  while (1) {
    // lightup_led(1);
    // nrf_delay_ms(1000);
    // lightup_led(2);
    // nrf_delay_ms(1000);
    // lightup_led(3);
    // nrf_delay_ms(1000);
    // lightup_led(0);
    // nrf_delay_ms(1000);
  }
}

