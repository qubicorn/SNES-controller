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

const char *TAG = "MAIN";

void app_main() {
    esp_log_level_set("*", ESP_LOG_ERROR);
    esp_log_level_set("SNES", ESP_LOG_DEBUG);
    esp_log_level_set("HTTP_CLIENT", ESP_LOG_DEBUG);

    nvs_flash_init();
    wifi_init();
    snes_init();
}