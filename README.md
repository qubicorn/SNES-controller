# SNES-controller
Embedding an [ESP32-s3](https://www.adafruit.com/product/5426) in an original SNES controller to use for wireless input in other projects.

SNES's original shift register is wired directly into the ESP32-s3. It's small enough to fit inside the controller (planned for later).

  ![ESP32-s3 SNES controller WIP](https://www.dropbox.com/s/k8kfzht682ct0vd/esp32-s3-snes-controller.jpg?dl=0)

Given the experimental nature, loosely defined objectives, and a lack of experience with **ESP32**, **ESP-IDF**, **PlatformIO**, **FreeRTOS**, I consider this a prototype and eschewed writing tests and some formalities.

## Working so far
- All SNES buttons reliably detected
- Handling of simultaneous button press "combos"
- - Registering callback functions on combos
  - Grouping combos so they can be enabled/disabled based on context
- Connects to WIFI configured in advance
  - Currently only used to send a demo POST request when a certain combo is detected

## Planned features
- Actually control things with it! Local webservers, IFTTT, etc.
- Headless modal interface via SNES button combos
- Pull configuration updates from webserver
- Haptic motors (rather than using a display or LEDs for feedback)
- Low power use, via heavy use of deep sleep and ULP coprocesser
- WiFi provisioning via BLE to a smartphone
- OTA firmware flashing
