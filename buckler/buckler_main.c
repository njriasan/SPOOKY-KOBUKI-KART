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

#define NUM_BUTTONS 4

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

states state = OFF;

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0x11, // TODO: replace with your lab bench number
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

// TODO: Declare control characteristic and variable for our service
static simple_ble_char_t controller_char = {.uuid16 = 0xeda1};
static uint16_t controller_bytes;

simple_ble_app_t* simple_ble_app;

// controls ordering: accelerate, decelerate, left, right
// This is incomplete, just a basic mapping to go forward and turn

// Buttons are X (accelerate), B (decelerate), Left, Right
static bool controls[NUM_BUTTONS] = {false, false, false, false};
static uint16_t masks[NUM_BUTTONS] = {0b1 << 3, 0b1, 0b11 << 10, 0b1 << 11};

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    // TODO: logic for each characteristic and related state changes
    // TODO: for the stick presses, we need to do equality checks instead
  for (unsigned int i = 0; i < NUM_BUTTONS; i++) {
    controls[i] = controller_bytes & masks[i];
  }

  // TODO: There's no rest state at the moment, we are just constantly decelerating but hitting th floor of 0
  if (controls[0] == true) {
    // Acclerating
    on_X_press();
  } else if (p_fsm.state == REST) {
    rest();
  } else {
    // Decelerate
    on_X_release();
  }

  if (controls[2] == true) {
    // Turning Left
    on_l_stick_press();
  } else if (controls[3] == true) {
    // Turning right
    on_r_stick_press();
  } else {
    // Go straight
    on_stick_release();
  }

  drive();
}

void print_state(states current_state){
	switch(current_state){
  	case OFF: {
  		display_write("OFF", DISPLAY_LINE_0);
  		break;
    }
    case ACCELERATE: {
      display_write("ACCELERATE", DISPLAY_LINE_0);
      break;
    }
    case DECELERATE: {
      display_write("DECELERATE", DISPLAY_LINE_0);
      break;
    }
    case LEFT: {
      display_write("LEFT", DISPLAY_LINE_0);
      break;
    }
    case RIGHT: {
      display_write("RIGHT", DISPLAY_LINE_0);
      break;
    }
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
  simple_ble_add_characteristic(1, 1, 0, 0, // read, write, notify, vlen
      sizeof(controller_bytes), (uint8_t*)&controller_bytes,
      &robot_service, &controller_char);

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

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    int status = kobukiSensorPoll(&sensors);

    // TODO: complete state machine
    switch(state) {
      case OFF: {
        print_state(state);

        // transition logic
        if (is_button_pressed(&sensors)) {
          // state = FORWARD;
        } else {
          state = OFF;
          // perform state-specific actions here
          kobukiDriveDirect(0, 0);
        }
        break; // each case needs to end with break!
      }

      case ACCELERATE: {
        print_state(state);

        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else {
          // perform state-specific actions here
          kobukiDriveDirect(100, 100);
        }
        break; // each case needs to end with break!
      }

      case DECELERATE: {
        print_state(state);

        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else {
          // perform state-specific actions here
          kobukiDriveDirect(-100, -100);
        }
        break; // each case needs to end with break!
      }

      case LEFT: {
        print_state(state);

        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else {
          // perform state-specific actions here
          kobukiDriveDirect(-50, 100);
        }
        break; // each case needs to end with break!
      }

      case RIGHT: {
        print_state(state);

        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else {
          // perform state-specific actions here
          kobukiDriveDirect(100, -50);
        }
        break; // each case needs to end with break!
      }
    }
  }
}
