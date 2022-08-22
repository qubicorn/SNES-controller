#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "wifi.h"


static const char* TAG = "MAIN";

void app_main() {
    // esp_log_level_set("*", ESP_LOG_WARN);
    // esp_log_level_set("this_module", ESP_LOG_INFO);

    nvs_flash_init();
    wifi_init(WIFI_SSID, WIFI_PASSWORD);

    while (1) {
        vTaskDelay(500 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "testing");
    }
}

