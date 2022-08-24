#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "wifi.h"
#include "snes.h"

#define GPIO_OUTPUT_IO_0    CONFIG_GPIO_OUTPUT_0
#define GPIO_OUTPUT_IO_1    CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static const char* TAG = "MAIN";

void app_main() {
    esp_log_level_set("*", ESP_LOG_WARN);
    gpio_init();    
    nvs_flash_init();
    
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("WIFI", ESP_LOG_WARN);
    esp_wifi_set_ps(WIFI_PS_NONE); // disable power saving
    wifi_init();
    
    esp_log_level_set("SNES", ESP_LOG_DEBUG);
    snes_init();
}

