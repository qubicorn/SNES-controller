# SNES-controller
Embedding an [ESP32-s3](https://www.adafruit.com/product/5426) in an original SNES controller to use for wireless input in other projects.

SNES's original shift register is wired directly into the ESP32-s3. It's small enough to fit inside the controller (planned for later).

![ESP32-s3 SNES controller WIP](https://ucbc7ad41ae18309d453a8df212a.previews.dropboxusercontent.com/p/thumb/ABrm6DWmLYcgorLsw8TiE0Qro4GitAtGgZ9QeaCAAG-0CxiwiQ6KEj0C5JHPPFytRZfq0m4u_MGuecTYOe2AI4aLW8RSKonEKPLsJJdj5FeuVEbeg9eKno1r3ziumloUgS2fU2Xha3RogrnsXdO32QHEI9Ab9gTD_UviihHmg6a8-D0vNQUAthl_JOqvU0Gbwv6l7TBvNm5a6Ab7UFxZvTjURJpklEexC7rHgBOOPLCUchWZwXKS0awpxs7jP35X_l-lkwHrb0-qpX_P5vyMG8RJ2mP1owiX11iQotcYKkibhkZxbgR09AgSkmo9F3bcUAbhc5OW1mqXpSNmnydj5Lc2AScTwXMAune1mLYlb7_QPQyTsXFLuJf8YWcHc4KB5wQj7RE9tcXZmqIEznFrA3mYwatqjwFZbVtMVZs9triuUw/p.jpeg)

## Planned features
- Headless interface via SNES button macros
- Haptic motors (rather than using a display or LEDs for feedback)
- Low power use, via heavy use of deep sleep and ULP coprocesser
- WiFi provisioning via BLE to a smartphone
- OTA firmware flashing
