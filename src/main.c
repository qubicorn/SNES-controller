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

void app_main() {
    esp_log_level_set("*", ESP_LOG_ERROR);
    //esp_log_level_set("WIFI", ESP_LOG_ERROR);
    esp_log_level_set("SNES", ESP_LOG_DEBUG);

    gpio_init();    
    nvs_flash_init();
    
    esp_wifi_set_ps(WIFI_PS_NONE); // disable power saving
    wifi_init();
    
    snes_init();
    // start snes task
    const uint32_t stackDepth = 5 * configMINIMAL_STACK_SIZE;
    TaskHandle_t xHandle;
    void *param = NULL;
    BaseType_t taskCreated;

    taskCreated = xTaskCreate(task_snes_read, "TASK_SNES", stackDepth, param, tskIDLE_PRIORITY, &xHandle);
    if (taskCreated != pdPASS) {
        ESP_LOGE("MAIN", "Failed to create snes task");
    }
}
