// Buckler Driver to connect to DWM1001C 

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_util.h"
#include "nrf_twi_mngr.h"
#include "nrf_gpio.h"
#include "dwm_driver.h"

#include "buckler.h"

#include "max44009.h"

int main(void) {

  // Initialize
  printf("Hello!\n");
  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  printf("2nd part\n");
  ret_code_t error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  printf("Error Check done.\n");
  uint8_t* readData = dwm_init(&spi_instance);
  //if (readData == NULL) {
  //  printf("ERROR!\n");
 // } else {
    //printf("%x%x%x\n", readData[0], readData[1], readData[2]);
  //}  
  readData = dwm_read_rate(&spi_instance);
  while(1) {
    power_manage();
  }
}
