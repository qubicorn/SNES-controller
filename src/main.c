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
//#define TASK_STACK_SIZE 4000

static const char* TAG = "MAIN";

#if 0
// Perform an action every 10 ticks.
void vTaskFunction(void * pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    BaseType_t xWasDelayed;

    xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // Wait for the next cycle.
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Perform action here. xWasDelayed value can be used to determine
        // whether a deadline was missed if the code here took too long.
        ESP_LOGI(TAG, "*action*");
    }
}

void vtask_test(void *pvParameters)
{
    ESP_LOGI(TAG, "*action*");
    vTaskDelete(NULL);
}
#endif

void app_main() {
    esp_log_level_set("*", ESP_LOG_WARN);
    gpio_init();    
    nvs_flash_init();
    
    esp_log_level_set("WIFI", ESP_LOG_WARN);
    esp_wifi_set_ps(WIFI_PS_NONE); // disable power saving
    wifi_init();
    
    esp_log_level_set("SNES", ESP_LOG_DEBUG);
    snes_init();

    //TaskHandle_t xHandle = NULL;
    //static uint8_t ucParameterToPass;
    // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    //xTaskCreate(vTaskFunction, "snes", TASK_STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
    //configASSERT( xHandle );
}
