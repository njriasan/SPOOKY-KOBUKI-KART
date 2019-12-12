// DWM1001C Driver

#pragma once
#include "app_error.h"
#include "nrf_drv_spi.h"

// Function that recieves positioning data from connected tag
// and converts it into coordinates. Must be called only after
// dwm_request_pos is called.
void update_dwm_pos(nrf_drv_spi_t* s, int32_t* location_bytes);

// Function to initialize the connected tag as a tag.
uint8_t* dwm_tag_init(nrf_drv_spi_t* s);

// Function to read the update rate from the connected tag.
uint8_t *dwm_read_rate(nrf_drv_spi_t *s);

// Function to read the position from the connected tag.
uint8_t *dwm_read_pos(nrf_drv_spi_t *s);

// Function to reset the connected tag.
bool dwm_reset(nrf_drv_spi_t *s);

// Function to issue a positioning request to the connected tag.
bool dwm_request_pos(nrf_drv_spi_t *s);

// Function to recieve the output of a positioning request from
// the connected tag. Must be called in tandem with the above 
// function. (After calling dwm_request_pos)
uint8_t *dwm_recieve_pos(nrf_drv_spi_t *s);
