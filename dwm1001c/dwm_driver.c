// DWM1001C Driver

#include <stdlib.h>
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "dwm_driver.h"

static nrf_drv_spi_t* spi;

uint8_t* dwm_init(nrf_drv_spi_t* s) {
  spi = s;
  uint8_t data[4];
  data[0] = 0x0A;
  data[1] = 0x02;
  data[2] = 0xD6;
  data[3] = 0x00;
  ret_code_t err_code = nrf_drv_spi_transfer(spi, data, 4, NULL, 0);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return NULL;  
  }
  uint8_t size_num[2];
  err_code = nrf_drv_spi_transfer(spi, NULL, 0, size_num, 2);
  while (size_num[0] == 0x00) {
    printf("%x %x\n", size_num[0], size_num[1]);
    APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
      return NULL;
    }
    nrf_delay_ms(10);
    err_code = nrf_drv_spi_transfer(spi, NULL, 0, size_num, 2);
  }
  printf("%x %x\n", size_num[0], size_num[1]);
  uint8_t* readData = (uint8_t *)malloc(sizeof(uint8_t)*size_num[0]);
  err_code = nrf_drv_spi_transfer(spi, NULL, 0, readData, size_num[0]);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return NULL;
  }
  printf("%x %x %x\n", readData[0], readData[1], readData[2]);
  return readData;
}

uint8_t *dwm_read_rate(nrf_drv_spi_t *s) {
  spi = s;
  uint8_t data[2];
  data[0] = 0x08;
  data[1] = 0x00;
  ret_code_t err_code = nrf_drv_spi_transfer(spi, data, 2, NULL, 0);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return NULL;  
  }
  uint8_t size_num[2];
  err_code = nrf_drv_spi_transfer(spi, NULL, 0, size_num, 2);
  while (size_num[0] == 0x00) {
    printf("%x %x\n", size_num[0], size_num[1]);
    APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
      return NULL;
    }
    nrf_delay_ms(10);
    err_code = nrf_drv_spi_transfer(spi, NULL, 0, size_num, 2);
  }
  printf("%x %x\n", size_num[0], size_num[1]);
  uint8_t* readData = (uint8_t *)malloc(sizeof(uint8_t)*size_num[0]);
  err_code = nrf_drv_spi_transfer(spi, NULL, 0, readData, size_num[0]);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return NULL;
  }
  int i = 0;
  while (i < size_num[0]) {
    printf("%x ", readData[i++]);
  }
  printf("\n");
  return readData;

}
