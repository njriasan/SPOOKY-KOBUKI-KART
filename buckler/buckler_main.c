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
#include "fsm.h"

#define NUM_BUTTONS 4

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:yy:xx:xx
        .platform_id       = 0x00,    // used as 4th octect in device BLE address yy
        .device_id         = 0x11, // TODO: replace with your lab bench number xx
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

typedef struct {
  char* name;
  uint16_t mask;
  uint8_t shift_amount;
  uint8_t value;
} button_info_t;

static button_info_t x_button = {"X", 0b1 << 3, 3, 0};
static button_info_t b_button = {"B", 0b1, 0, 0};
static button_info_t stick_push_button = {"STICK PUSH", 0b1111 << 8, 8, 8};

static button_info_t *buttons[NUM_BUTTONS] = {&x_button, &b_button, &stick_push_button };

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    // TODO: logic for each characteristic and related state changes
  //printf("%x\n", stick_push_button.value);
  for (unsigned int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i]->value = (buttons[i]->mask & controller_bytes) >> buttons[i]->shift_amount;
  }
    //uint8_t *bytes_look = (uint8_t *) &controller_bytes;
    //printf("%x\n", stick_push_button.value);
  	//printf("%x %x\n", bytes_look[0], bytes_look[1]);
  	//printf("\n\n");

  // TODO: There's no rest state at the moment, we are just constantly decelerating but hitting th floor of 0
  if (x_button.value == 1) {
    // Acclerating
    on_X_press();
  } else if (p_fsm.state == REST) {
    rest();
  } else {
    // Decelerate
    on_X_release();
  }

  if (stick_push_button.value == 6) {
    // Turning Left
    on_l_stick_press();
  } else if (stick_push_button.value == 2) {
    // Turning right
    on_r_stick_press();
  } else {
    // Go straight
    on_stick_release();
  }

}

void print_power_state(power_states current_state){
	switch(current_state){
	  	case REST: {
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
	}
}

void print_turning_state(turning_states current_state){
	switch(current_state){
	  	case LEFT: {
	  		display_write("LEFT", DISPLAY_LINE_1);
	  		break;
	    }
	    case CENTER: {
	      display_write("CENTER", DISPLAY_LINE_1);
	      break;
	    }
	    case RIGHT: {
	      display_write("RIGHT", DISPLAY_LINE_1);
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

  // Initialize the fsms
  init_power_fsm(&p_fsm);
  init_turning_fsm(&t_fsm);

  // loop forever, running state machine
  while (1) {
	  	// TODO: There's no rest state at the moment, we are just constantly decelerating but hitting th floor of 0
	  if (x_button.value == 1) {
	    // Acclerating
	    on_X_press();
	  } else if (p_fsm.state == REST) {
	    rest();
	  } else {
	    // Decelerate
	    on_X_release();
	  }

	  if (stick_push_button.value == 6) {
	    // Turning Left
	    on_l_stick_press();
	  } else if (stick_push_button.value == 2) {
	    // Turning right
	    on_r_stick_press();
	  } else {
	    // Go straight
	    on_stick_release();
	  }
  	// Drive the kobuki
  	drive();

  	print_power_state(p_fsm.state);
  	print_turning_state(t_fsm.state);
  	/* May read sensors later. */
    // read sensors from robot
    //int status = kobukiSensorPoll(&sensors);
  }
}

