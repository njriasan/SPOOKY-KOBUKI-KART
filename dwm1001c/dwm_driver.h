// DWM1001C Driver

#pragma once
#include "app_error.h"
#include "nrf_drv_spi.h"

uint8_t* dwm_init(nrf_drv_spi_t* s);
uint8_t *dwm_read_rate(nrf_drv_spi_t *s);
