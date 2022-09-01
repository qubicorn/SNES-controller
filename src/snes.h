#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

#define PIN_SNES_LATCH CONFIG_SNES_LATCH
#define PIN_SNES_CLOCK CONFIG_SNES_CLOCK
#define PIN_SNES_DATA  CONFIG_SNES_DATA

#define SNES_REGISTER_DEFAULT 0xfff 
#define SNES_NUM_BUTTONS 12
#define SNES_REGISTER_NUM_BITS SNES_NUM_BUTTONS
#define SNES_POLLILNG_RATE CONFIG_SNES_POLLILNG_RATE
#define SNES_LATCH_PULSE_WIDTH CONFIG_SNES_LATCH_PULSE_WIDTH

#define SNES_TASK_STACK_SIZE 4096

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

extern int snes_register;

void snes_init();
void snes_debug_print_register(int snes_register);
int IRAM_ATTR snes_read_controller();
void IRAM_ATTR task_snes_read();