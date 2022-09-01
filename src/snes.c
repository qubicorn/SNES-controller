#include "snes.h"

static const char *TAG = "SNES";

// low stack space warning
static const int MIN_STACK_ALARM_TIMEOUT = 1000;
int minStackAlarmTimeout = 0;
bool debug = false;

const char *SNES_BUTTON_LABELS[] = {
  "B",
  "Y",
  "SELECT",
  "START",
  "UP",
  "DOWN",
  "LEFT",
  "RIGHT",
  "A",
  "X",
  "LT",
  "RT"
};

const gpio_config_t io_confs[] = {
  {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_LATCH,
  },
  {
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_CLOCK,
  },
  {
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = 1ULL << PIN_SNES_DATA,
    .pull_up_en = GPIO_PULLUP_ENABLE,
  },
};

int snes_register = SNES_REGISTER_DEFAULT;



void IRAM_ATTR pulse_latch()
{
    ESP_ERROR_CHECK(gpio_set_level(PIN_SNES_LATCH, 1));
    vTaskDelay(1 / portTICK_RATE_MS);
    ESP_ERROR_CHECK(gpio_set_level(PIN_SNES_LATCH, 0));
    vTaskDelay(1 / portTICK_RATE_MS);
}

void IRAM_ATTR pulse_clock()
{
    // shift register to next bit to be read
    ESP_ERROR_CHECK(gpio_set_level(PIN_SNES_CLOCK, 1));
    ESP_ERROR_CHECK(gpio_set_level(PIN_SNES_CLOCK, 0));
}

char *register_to_binary(int snes_register, char *bin_snes_register)
{
    /**
     * OUT
     * bin_snes_register: string containing binary of _snes_register
     */
    for (int i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
        bin_snes_register[i] = snes_register & 1 ? '1' : '0';
        snes_register >>= 1;
    }
    bin_snes_register[SNES_REGISTER_NUM_BITS] = '\0';
    return bin_snes_register;
}

void snes_debug_print_register(int snes_register)
{
    // print entire register
    char bin_snes_register[SNES_NUM_BUTTONS + 1];
    register_to_binary(snes_register, bin_snes_register);
    ESP_LOGI(TAG, "controller register: %s", bin_snes_register);

    // print labels of pressed buttons
    const int TERM_RED = 31;
    for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
        if ((snes_register >> i & 1) == 0) {
            ESP_LOGI(TAG, "\e[%im%s\e[0m", TERM_RED,
                SNES_BUTTON_LABELS[i]
            );
        }
    }
}

int IRAM_ATTR snes_read_controller()
{
    // latch to capture state and then clock pulse to retrieve each bit
    int new_register = SNES_REGISTER_DEFAULT;
    for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
        i == 0 ? pulse_latch() : pulse_clock();
        if (!gpio_get_level(PIN_SNES_DATA)) {
            new_register &= ~(1ULL << i);
        }
    }

    if (new_register == 0) {
        ESP_LOGW(TAG, "no data signal coming from yellow SNES controller wire");
        return 0;
    }

    // debug print any detected button presses
    if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG && new_register != SNES_REGISTER_DEFAULT) {
        snes_debug_print_register(new_register);
    }
    return new_register;
}


void IRAM_ATTR task_snes_read() {
    while (true) {
        int snes_register = snes_read_controller();
        int stackSpace = uxTaskGetStackHighWaterMark(NULL);
        if (stackSpace < SNES_MIN_FREE_STACK_SIZE) {
            if (minStackAlarmTimeout-- <= 0) {
                minStackAlarmTimeout = MIN_STACK_ALARM_TIMEOUT;
                ESP_LOGW(TAG, "available stack space is low (%d)", stackSpace);
            }
        };

        // Debug combos
        if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG && snes_register != SNES_REGISTER_DEFAULT) {
            int stack_combo = 1<<SNES_BTN_RT | 1<<SNES_BTN_LT;
            if ((~snes_register & stack_combo) == stack_combo) {
                ESP_LOGI(TAG, "available stack space: %d", stackSpace);

                //poor man's debounce
                vTaskDelay(50);
                continue;
            }

            int debug_combo = 1<<SNES_BTN_START;
            if ((~snes_register & debug_combo) == debug_combo) {
                debug = !debug;
                ESP_LOGI(TAG, "debug mode: %s", (debug ? "ON" : "OFF"));

                //poor man's debounce
                vTaskDelay(50);
                continue;
            }
        }
    }
}

void start_task() {
    TaskHandle_t xHandle;
    void *param = NULL;
    BaseType_t taskCreated;

    ESP_LOGI(TAG, "creating SNES task with %d bytes of memory", SNES_TASK_STACK_SIZE);
    taskCreated = xTaskCreate(task_snes_read, "TASK_SNES", SNES_TASK_STACK_SIZE, param, tskIDLE_PRIORITY, &xHandle);
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "Failed to create snes task");
    }
}

void gpio_init()
{
    size_t num_gpio_confs = sizeof(io_confs) / sizeof(gpio_config_t);
    for (int i = 0; i < num_gpio_confs; ++i) {
        ESP_ERROR_CHECK(gpio_config(&io_confs[i]));
    }
    ESP_LOGI(TAG, "gpio ports initialized");
}

void snes_init()
{
    gpio_init();
    start_task();
}
