#include "snes.h"

void snes_gpio_init() {
    size_t num_gpio_confs = sizeof(io_confs)/sizeof(gpio_config_t);
    for (int i=0; i < num_gpio_confs; ++i) {
        gpio_config(&io_confs[i]);
    }
}

void snes_pulse_latch() {
  // initiate retrieval of current button states
  digitalWrite(PIN_SNES_LATCH, 1);
  delayMicroseconds(1);
  digitalWrite(PIN_SNES_LATCH, 0);
}

void snes_pulse_clock() {
  // shift register to next bit to be read
  digitalWrite(PIN_SNES_CLOCK, 1);
  digitalWrite(PIN_SNES_CLOCK, 0);
}

int snes_read_controller() {
  int ret = SNES_REGISTER_DEFAULT;
  for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
    i == 0 ? snes_pulse_latch() : snes_pulse_clock();
    if (digitalRead(PIN_SNES_DATA) == 0)
      bitClear(ret, i);
  }

#if DEBUG
  // if any button presses are found, print the whole register
  if (ret != SNES_REGISTER_DEFAULT) {
    Serial.print("SNES controller register: ");
    Serial.print(ret, BIN);
    Serial.println();
  }
  if (ret == 0) {
    Serial.println("Warning: no data coming from yellow SNES controller wire");
  }
#endif

  return ret;
}