#include "snes.h"

static const char* TAG = "SNES";

void snes_gpio_init() {
    size_t num_gpio_confs = sizeof(io_confs)/sizeof(gpio_config_t);
    for (int i=0; i < num_gpio_confs; ++i) {
        gpio_config(&io_confs[i]);
    }
    ESP_LOGI(TAG, "gpio ports initialized");
}

void snes_pulse_latch() {
  // initiate retrieval of current button states
  //taskENTER_CRITICAL(&mutex);
      gpio_set_level(PIN_SNES_LATCH, 1);
      vTaskDelay(1);  // FIXME: delay can be much shorter, was 1us before
      gpio_set_level(PIN_SNES_LATCH, 0);
  //taskEXIT_CRITICAL(&mutex);
}

void snes_pulse_clock() {
  // shift register to next bit to be read
  //taskENTER_CRITICAL(&mutex);
      gpio_set_level(PIN_SNES_CLOCK, 1);
      gpio_set_level(PIN_SNES_CLOCK, 0);
  //taskEXIT_CRITICAL(&mutex);
}

char * snes_register_to_binary(int snes_register, char *bin_snes_register) {
  /**
   * OUT
   * bin_snes_register: string containing binary of _snes_register
   */
  for (int i=0; i < SNES_REGISTER_NUM_BITS; ++i) {
    bin_snes_register[i] = snes_register & 1 ? '1' : '0';
    snes_register >>= 1;
  }
  bin_snes_register[SNES_REGISTER_NUM_BITS] = '\0';
  return bin_snes_register;
}

int snes_read_controller() {
  int ret = SNES_REGISTER_DEFAULT;
  for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
    i == 0 ? snes_pulse_latch() : snes_pulse_clock();
    if (!gpio_get_level(PIN_SNES_DATA)) {
      ret &= ~(1ULL << i);
    }
  }

  if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
      // if any button presses are found, print the whole register
    if (ret != SNES_REGISTER_DEFAULT) {
      char bin_snes_register[SNES_NUM_BUTTONS+1];
      snes_register_to_binary(ret, bin_snes_register);
      ESP_LOGI(TAG, "controller register: %s", bin_snes_register);
    }
    if (ret == 0) {
      ESP_LOGW(TAG, "no data signal coming from yellow SNES controller wire");
    }
  }

  return ret;
}

void snes_loop() {
  // 12-bit bitmask for SNES buttons states; HIGH (not pushed) or LOW (pushed)
  snes_register = snes_read_controller();
  if (snes_register == SNES_REGISTER_DEFAULT) {
    return;
  }
  last_pressed_register = snes_register;
  
  // print labels for pressed buttons
  if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
    const int TERM_RED = 31;
    for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
      if ((snes_register >> i & 1) == 0) {
        ESP_LOGI(TAG, "\e[%im%s\e[0m", TERM_RED, SNES_BUTTON_LABELS[i]);
      }
    }
  }
}