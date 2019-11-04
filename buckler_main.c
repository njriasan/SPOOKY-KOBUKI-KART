// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"
#include "simple_ble.h"

#include "states.h"

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0x000C, // TODO: replace with your lab bench number
        .adv_name          = "KOBUKI", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
static simple_ble_service_t robot_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

// TODO: Declare characteristics and variables for your service
static simple_ble_char_t forward_char = {.uuid16 = 0xeda1};
static bool forward_state = false;
static simple_ble_char_t back_char = {.uuid16 = 0xeda2};
static bool back_state = false;
static simple_ble_char_t left_char = {.uuid16 = 0xeda3};
static bool left_state= false;
static simple_ble_char_t right_char = {.uuid16 = 0xeda4};
static bool right_state= false;




simple_ble_app_t* simple_ble_app;

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    // TODO: logic for each characteristic and related state changes
  if (simple_ble_is_char_event(p_ble_evt, &forward_char)) {
    printf("Got write to LCD characteristic!\n");
    // display_write("FORWARD", DISPLAY_LINE_1);
  }
  if (simple_ble_is_char_event(p_ble_evt, &back_char)) {
    printf("Got write to LCD characteristic!\n");
    // display_write("BACK", DISPLAY_LINE_1);
    // kobukiDriveDirect(-100,-100);
  }
  if (simple_ble_is_char_event(p_ble_evt, &left_char)) {
    printf("Got write to LCD characteristic!\n");
    // display_write("LEFT", DISPLAY_LINE_1);
    // kobukiDriveDirect(-100,100);
  }
  if (simple_ble_is_char_event(p_ble_evt, &right_char)) {
    printf("Got write to LCD characteristic!\n");
    // display_write("RIGHT", DISPLAY_LINE_1);
    // kobukiDriveDirect(100,-100);
  }
}

void print_state(states current_state){
	switch(current_state){
	case OFF:
		display_write("OFF      ", DISPLAY_LINE_0);
		break;
  case ON:
    display_write("ON       ", DISPLAY_LINE_0);
    break;
  case FORWARD:
    display_write("FORWARD ", DISPLAY_LINE_0);
    break;
  case BACK:
    display_write("BACK     ", DISPLAY_LINE_0);
    break;
  case LEFT:
    display_write("LEFT     ", DISPLAY_LINE_0);
    break;
  case RIGHT:
    display_write("RIGHT    ", DISPLAY_LINE_0);
    break;
    }
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&robot_service);

  // TODO: Register your characteristics

  // Start Advertising
  simple_ble_adv_only_name();

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

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
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  states state = OFF;
  simple_ble_add_characteristic(1, 1, 0, 0,
        sizeof(forward_state), (uint8_t*)&forward_state,
        &robot_service, &forward_char);
  simple_ble_add_characteristic(1, 1, 0, 0,
        sizeof(back_state), (uint8_t*)&back_state,
        &robot_service, &back_char);
  simple_ble_add_characteristic(1, 1, 0, 0,
        sizeof(left_state), (uint8_t*)&left_state,
        &robot_service, &left_char);
  simple_ble_add_characteristic(1, 1, 0, 0,
        sizeof(right_state), (uint8_t*)&right_state,
        &robot_service, &right_char);

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    int status = kobukiSensorPoll(&sensors);
    printf("%u\n", right_state);
    // TODO: complete state machine
    switch(state) {
      case OFF: {
        print_state(state);

        // transition logic
        if (is_button_pressed(&sensors)) {
          state = ON;
        } else {
          state = OFF;
          // perform state-specific actions here
          kobukiDriveDirect(0, 0);
        }
        break; // each case needs to end with break!
      }
      case ON: {
        print_state(state);
        // transition logic
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (forward_state) {
          state = FORWARD;
        } else if (back_state) {
          state = BACK;
        } else if (left_state) {
          state = LEFT;
        } else if (right_state) {
          state = RIGHT;
        } else {
          state = ON;
          // perform state-specific actions here
        }
        break;
      }
      case FORWARD : {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (back_state) {
          state = BACK;
        } else if (left_state) {
          state = LEFT;
        } else if (right_state) {
          state = RIGHT;
        } else {
          kobukiDriveDirect(100,100);
        }
        break;
      }
      case BACK : {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (forward_state) {
          state = FORWARD;
        } else if (left_state) {
          state = LEFT;
        } else if (right_state) {
          state = RIGHT;
        } else {
          kobukiDriveDirect(-100,-100);
        }
        break;
      }
      case LEFT : {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (back_state) {
          state = BACK;
        } else if (forward_state) {
          state = FORWARD;
        } else if (right_state) {
          state = RIGHT;
        } else {
          kobukiDriveDirect(-100,100);
        }
        break;
      }
      case RIGHT : {
        print_state(state);
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (back_state) {
          state = BACK;
        } else if (left_state) {
          state = LEFT;
        } else if (forward_state) {
          state = FORWARD;
        } else {
          kobukiDriveDirect(100,-100);
        }
        break;
      }
    }
  }
}

