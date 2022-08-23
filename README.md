# SNES-controller
Embedding an [ESP32-s3](https://www.adafruit.com/product/5426) in an original SNES controller to use for wireless input to other projects.

SNES's original shift register is wired directly into the ESP32-s3. It's small enough to fit inside the controller.

## Planned features
- Headless interface via SNES button macros
- Haptic motors (rather than using a display or LEDs for feedback)
- Low power use, via heavy use of deep sleep and ULP coprocesser
- WiFi provisioning via BLE to a smartphone
- OTA firmware flashing
