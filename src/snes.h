#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_SNES_LATCH CONFIG_SNES_LATCH
#define PIN_SNES_CLOCK CONFIG_SNES_CLOCK
#define PIN_SNES_DATA  CONFIG_SNES_DATA

#define SNES_DEBOUNCE_MS CONFIG_SNES_DEBOUNCE_MS
#define SNES_REGISTER_DEFAULT 0xfff 
#define SNES_NUM_BUTTONS 12
#define SNES_REGISTER_NUM_BITS SNES_NUM_BUTTONS

// SNES controller bits
enum snes_btn {
  SNES_BTN_B,
  SNES_BTN_Y,
  SNES_BTN_SELECT,
  SNES_BTN_START,
  SNES_BTN_UP,
  SNES_BTN_DOWN,
  SNES_BTN_LEFT,
  SNES_BTN_RIGHT,
  SNES_BTN_A,
  SNES_BTN_X,
  SNES_BTN_LT,
  SNES_BTN_RT
};
static const char *SNES_BUTTON_LABELS[] = {
  "B",
  "Y",
  "SELECT",
  "START",
  "UP",
  "DOWN",
  "LEFT",
  "RIGHT",
  "A",
  "X",
  "LT",
  "RT"
};

static int snes_register = SNES_REGISTER_DEFAULT;  // holds current button states
static int last_pressed_register = SNES_REGISTER_DEFAULT;  // last buttons pressed
static gpio_config_t io_confs[] = {
  {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_LATCH,
  },
  {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_CLOCK,
  },
  {
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_DATA,
    .pull_up_en = GPIO_PULLUP_ENABLE,
  },
};

void snes_gpio_init();
void snes_pulse_latch();
void snes_pulse_clock();
int snes_read_controller();
char * snes_register_to_binary(int snes_register, char *bin_snes_register);
void snes_loop();