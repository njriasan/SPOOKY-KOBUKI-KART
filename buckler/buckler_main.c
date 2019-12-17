// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_error.h"
#include "app_timer.h"
#include "app_util.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_twi_mngr.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"
#include "simple_ble.h"
#include "dwm_driver.h"
#include "max44009.h"

#include "states.h"
#include "fsm.h"
#include "powerups.h"
#include "led.h"

#define NUM_BUTTONS 4

void controller_evt_write();
void powerup_evt_write();
void hazard_evt_write();


// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
  // c0:98:e5:yy:xx:xx
  .platform_id       = 0x00,          // used as 4th octect in device BLE address yy
  .device_id         = 0x13,       // TODO: replace with your lab bench number xx
  .adv_name          = "KOBUKI",       // used in advertisements if there is room
  .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
  .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
  .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

// 4607eda0-f65e-4d59-a9ff-84420d87a4ca
static simple_ble_service_t robot_service = {{
                                               .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                                                           0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x56}
                                             }};

// TODO: Declare control characteristic and variable for our service
#define CONTROLLER_UUID 0xeda1
static simple_ble_char_t controller_char = {.uuid16 = CONTROLLER_UUID};
static uint16_t controller_bytes         = 0;

#define POWERUP_UUID 0xeda2
static simple_ble_char_t powerup_char = {.uuid16 = 0xeda2};
static uint8_t powerup_byte = NO_POWERUP;

#define HAZARD_UUID 0xeda3
static simple_ble_char_t hazard_char = {.uuid16 = 0xeda3};
static uint8_t hazard_byte = NO_HAZARD;

static simple_ble_char_t location_char = {.uuid16 = 0xeda4};
static int32_t location_bytes[3]       = {0, 0, 0};

simple_ble_char_t shell_char = {.uuid16 = 0xeda5};
uint8_t shell_byte = NO_SHELL_BYTE;

simple_ble_app_t* simple_ble_app;

// controls ordering: accelerate, decelerate, left, right

static button_info_t x_button          = {"X", 0b1 << 3, 3, 0};
static button_info_t b_button          = {"B", 0b1, 0, 0};
button_info_t rz_button         = {"RZ", 0b1 << 4, 4, 0};
static button_info_t stick_push_button = {"STICK PUSH", 0b1111 << 8, 8, 8};

static button_info_t* buttons[NUM_BUTTONS] = {&x_button, &b_button, &rz_button, &stick_push_button};

void ble_evt_write(ble_evt_t const* p_ble_evt) {
  switch (p_ble_evt->evt.gatts_evt.params.write.uuid.uuid) {
    case (CONTROLLER_UUID):
    {
      controller_evt_write();
      break;
    }
    case (POWERUP_UUID):
    {
      powerup_evt_write();
      break;
    }
    case (HAZARD_UUID):
    {
      hazard_evt_write();
      break;
    }
    default: {
      break;
    }
  }
}

void controller_evt_write() {
  for (unsigned int i = 0; i < NUM_BUTTONS; i++) {
    if (buttons[i] == &rz_button) {
      rz_backup = (buttons[i]->mask & controller_bytes) >> buttons[i]->shift_amount;
      if (powerup_value == NO_POWERUP || buttons[i]->value == 0) {
        buttons[i]->value = (buttons[i]->mask & controller_bytes) >> buttons[i]->shift_amount;

      }

    } else {
      buttons[i]->value = (buttons[i]->mask & controller_bytes) >> buttons[i]->shift_amount;
    }
  }
  //printf("controller_bytes %x\n", controller_bytes);
  controller_bytes = 0;
}

// Update the powerup value only if there is no existing powerup
void powerup_evt_write() {
  printf("Powerup value %d\n", powerup_byte);
  if (powerup_value == NO_POWERUP && hazard_value == NO_HAZARD) {
    if (powerup_byte == MUSHROOM_POWERUP || powerup_byte == REDSHELL_POWERUP || powerup_byte == BLUESHELL_POWERUP) {
      powerup_value = powerup_byte;
      // Add information about setting the lights
      if (powerup_byte == MUSHROOM_POWERUP) {
        lightup_led(2);
      } else if (powerup_byte == REDSHELL_POWERUP) {
        //printf("Received redshell\n");
        lightup_led(1);
      }
    }
  }
  powerup_byte = NO_POWERUP;
}

// Update the hazard value only if there is no existing hazard
void hazard_evt_write() {
  //printf("Hazard value %d\n", hazard_byte);
  if (hazard_value == NO_HAZARD) {
    if (hazard_byte == BANANA_HAZARD || hazard_byte == REDSHELL_HAZARD || hazard_byte == BLUESHELL_HAZARD) {
      hazard_value = hazard_byte;
      //printf("Hazard Received %d\n", hazard_byte);

      if (hazard_byte == BANANA_HAZARD) {
        lightup_led(4);
      } else if (hazard_byte == REDSHELL_HAZARD) {
        lightup_led(3);
      }
    }
  }
  hazard_byte = NO_HAZARD;
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

  // Register your characteristics

  // Characteristic for button presses
  simple_ble_add_characteristic(1, 1, 0, 0, // read, write, notify, vlen
                                sizeof(controller_bytes), (uint8_t*)&controller_bytes,
                                &robot_service, &controller_char);

  // Characteristic for powerup receiving
  simple_ble_add_characteristic(1, 1, 0, 0, // read, write, notify, vlen
                                sizeof(powerup_byte), (uint8_t*)&powerup_byte,
                                &robot_service, &powerup_char);

  // Characteristic for hazard receiving
  simple_ble_add_characteristic(1, 1, 0, 0, // read, write, notify, vlen
                                sizeof(hazard_byte), (uint8_t*)&hazard_byte,
                                &robot_service, &hazard_char);

  // Characteristic for location sending
  simple_ble_add_characteristic(1, 0, 1, 0, // read, write, notify, vlen
                                sizeof(location_bytes), (uint8_t*)location_bytes,
                                &robot_service, &location_char);

  // Characteristic for location sending
  simple_ble_add_characteristic(1, 0, 1, 0, // read, write, notify, vlen
                                sizeof(shell_byte), (uint8_t*)&shell_byte,
                                &robot_service, &shell_char);

  // Start Advertising
  simple_ble_adv_only_name();

  // // initialize display or dwm
  nrf_drv_spi_t spi_instance      = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin      = BUCKLER_LCD_SCLK,
    .mosi_pin     = BUCKLER_LCD_MISO,
    .miso_pin     = BUCKLER_LCD_MOSI,
    .ss_pin       = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc       = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode      = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  printf("2nd part\n");
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  printf("Error Check done.\n");

  uint8_t* readData = dwm_tag_init(&spi_instance);
  while (readData[0] != 0x40 || readData[2] != 0x00) {
    printf("Config errored!");
    free(readData);
    readData = dwm_tag_init(&spi_instance);
  }
  while (!dwm_reset(&spi_instance)) {
    printf("Resetting");
  }
  printf("dwm tag initialized!\n");

  // display_init(&spi_instance);
  // printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl       = BUCKLER_SENSORS_SCL;
  i2c_config.sda       = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

  // initialize LED
  led_init();
  lightup_led(0);

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  // Initialize the fsms
  timer_init();
  init_velocity_fsm(&v_fsm);
  init_turning_fsm(&t_fsm);

  // nrf_delay_ms(100);
  // Launch a new thread to run the dwm code

  // loop forever, running state machine
  uint32_t timer_prev = read_timer();
  uint32_t timer_curr;
  bool shouldPollPos = false;
  while (1) {
    timer_curr = read_timer();
    // Update location roughly every 1/10 of a second
    if ((shouldPollPos = get_time_elapsed (timer_prev, timer_curr) > 0.5)) {
      timer_prev = timer_curr;
      while (!dwm_request_pos(&spi_instance)) {;
      }
    }
    if (!active_hazard) {
      if (active_powerup) {
        // Add logic for checking if the powerup has expired
        compare_time = read_timer();
        if (get_time_elapsed(powerup_starttime, compare_time) > powerup_duration) {
          decay_mushroom();
          // printf("%s\n", "powerup ended");
        } else {
          v_fsm.t_curr = read_timer();
        }
      } else if (v_fsm.state == MUSHROOM_DECAY) {
        decay_mushroom();
        // Add logic for the button press here
      } else if (powerup_value != NO_POWERUP && rz_button.value == 1) {
        // printf("%s%d\n", "powerup_value is ", powerup_value);
        if (powerup_value == MUSHROOM_POWERUP) {
          apply_mushroom();
          // nrf_delay_ms(100);
        } else if (powerup_value == REDSHELL_POWERUP) {
          apply_redshell_powerup();
        } else if (powerup_value == BLUESHELL_POWERUP) {
          apply_blueshell_powerup();
        }
      } else if (x_button.value == 1) {
        if (v_fsm.state == EXIT_POWERUP) {
          // printf("Transitioned to accelerating with velocity %lf\n", v_fsm.v);
        }
        // Acclerating
        on_X_press();
      } else if (b_button.value == 1) {
        // Braking/reversing
        on_B_press();
      } else if (v_fsm.state == REST) {
        rest();
      } else {
        // Cruising
        on_button_release();
      }
    }
    if (active_hazard) {
      compare_time = read_timer();
      if (get_time_elapsed(hazard_starttime, compare_time) > hazard_duration) {
        decay_hazard();
      } else {
        v_fsm.t_curr = read_timer();
      }
    } else if (hazard_value == BANANA_HAZARD) {
      apply_banana();
    } else if (hazard_value == REDSHELL_HAZARD) {
      apply_redshell_hazard();
    } else if (hazard_value == BLUESHELL_HAZARD) {
      apply_blueshell_hazard();
    } else if (stick_push_button.value == 6) {
      // Turning left
      on_l_stick_press();
    } else if (stick_push_button.value == 2) {
      // Turning right
      on_r_stick_press();
    } else if (stick_push_button.value == 7) {
      // Turning left diagonal
      on_l_up_stick_press();
    } else if (stick_push_button.value == 1) {
      // Turning right diagonal
      on_r_up_stick_press();
    } else if (stick_push_button.value == 5) {
      // Turning "right" backwards
      on_r_stick_press();
    } else if (stick_push_button.value == 3) {
      // Turning "left" backwards
      on_l_stick_press();
    } else {
      // Go straight
      on_stick_release();
    }
    // Drive the Kobuki
    drive();
    // printf("v: %lf\n", v_fsm.v);
    if (shouldPollPos) {
      update_dwm_pos(&spi_instance, location_bytes);
      simple_ble_notify_char(&location_char);
    }

    // Check if we need to try resend a shell notification
    if (shell_not_notified) {
      uint32_t err_code = simple_ble_notify_char(&shell_char);
      if (err_code == NRF_SUCCESS) {
        shell_not_notified = false;
      }
    }

    // print_velocity_state(v_fsm.state);
    // print_turning_state(t_fsm.state);
    /* May read sensors later. */
    // read sensors from robot
    // int status = kobukiSensorPoll(&sensors);
    // nrf_delay_ms(100);
  }
}

