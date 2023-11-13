#include "snes.h"

#define PIN_SNES_LATCH CONFIG_SNES_LATCH
#define PIN_SNES_CLOCK CONFIG_SNES_CLOCK
#define PIN_SNES_DATA  CONFIG_SNES_DATA

#define SNES_REGISTER_DEFAULT 0xfff 
#define SNES_NUM_BUTTONS 12
#define SNES_REGISTER_NUM_BITS SNES_NUM_BUTTONS
#define SNES_POLLILNG_RATE CONFIG_SNES_POLLILNG_RATE
#define SNES_LATCH_PULSE_WIDTH CONFIG_SNES_LATCH_PULSE_WIDTH

//#define SNES_TASK_STACK_SIZE 4096
#define SNES_TASK_STACK_SIZE 8192
#define SNES_MIN_FREE_STACK_SIZE 1024


static const char *TAG = "SNES";

// Each button combo group has a set of combos with function callbacks
#define COMBO_GROUP_MAIN 0
#define COMBO_GROUP_DEBUG 1
#define COMBO_GROUPS_MAX 5 // max number of groups
#define COMBOS_MAX 20 // max number of combos per group
static int combo_groups_active = 0;
static int combo_count[COMBO_GROUPS_MAX]; // current number of combos in each group
static int combo_masks[COMBO_GROUPS_MAX][COMBOS_MAX];

// combo callbacks
typedef void (*f_snes_btn_combo)(void);
static f_snes_btn_combo f_combos[COMBO_GROUPS_MAX][COMBOS_MAX];

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

// low stack space warning
static const int MIN_STACK_ALARM_TIMEOUT = 1000;
int minStackAlarmTimeout = 0;

// SNES controller bits
enum snes_btn {
  SNES_BTN_B,
  SNES_BTN_Y,
  SNES_BTN_SELECT,
  SNES_BTN_START,
  SNES_BTN_UP,
  SNES_BTN_DOWN,
  SNES_BTN_LEFT,
  SNES_BTN_RIGHT,
  SNES_BTN_A,
  SNES_BTN_X,
  SNES_BTN_LT,
  SNES_BTN_RT
};
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

int IRAM_ATTR read_controller()
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

void check_free_stack_space() {
    int stackSpace = uxTaskGetStackHighWaterMark(NULL);
    if (stackSpace < SNES_MIN_FREE_STACK_SIZE) {
        if (minStackAlarmTimeout-- <= 0) {
            minStackAlarmTimeout = MIN_STACK_ALARM_TIMEOUT;
            ESP_LOGW(TAG, "available stack space is low (%d)", stackSpace);
        }
    };
}

void debug_toggle() {
    combo_groups_active ^= 1<<COMBO_GROUP_DEBUG;
    bool is_debug = (combo_groups_active & 1<<COMBO_GROUP_DEBUG);
    ESP_LOGI(TAG, "debug mode: %s", is_debug ? "ON" : "OFF");
    http_post_kv("debug_mode", (int)is_debug);

}

void debug_stack() {
    int stackSpace = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "available stack space: %d", stackSpace);
}

void add_combo(int group, int btn_mask, f_snes_btn_combo cb) {
    int combo = combo_count[group];
    if (group >= COMBO_GROUPS_MAX) {
        ESP_LOGE(TAG, "tried to use group id %d but max is %d",
            group,
            COMBO_GROUPS_MAX - 1
        );
        return;
    }
    if (combo >= COMBOS_MAX) {
        char bin[SNES_REGISTER_NUM_BITS];
        ESP_LOGE(TAG, "attempted to add too many combos (group: %d, btn_mask: %s)",
            group, register_to_binary(combo, bin)
        );
        return;
    }

    combo_masks[group][combo] = btn_mask;
    f_combos[group][combo] = cb;
    ++combo_count[group];
}

void enable_combo_group(int group_mask) {
    combo_groups_active |= 1<<group_mask;
}

void disable_combo_group(int group_mask) {
    combo_groups_active &= ~(1<<group_mask);
}

void combo_detect(int snes_register) {
    if (snes_register == SNES_REGISTER_DEFAULT) {
        return;
    }

    for (int group=0; group < COMBO_GROUPS_MAX; ++group) {
        if ((combo_groups_active & 1<<group) == 0) continue;
        if (combo_count[group] == 0) continue;

        for (int combo=0; combo < combo_count[group]; ++combo) {
            int combo_mask = combo_masks[group][combo];
            if ((~snes_register & combo_mask) == combo_mask) {
                (*f_combos[group][combo])();
                vTaskDelay(50); // debounce
            }
        }
    }
}

void IRAM_ATTR task_read_input() {
    while (true) {
        check_free_stack_space();
        int snes_register = read_controller();
        combo_detect(snes_register);
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

void combo_init()
{
    add_combo(COMBO_GROUP_MAIN, 1<<SNES_BTN_START | 1<<SNES_BTN_UP, debug_toggle);
    add_combo(COMBO_GROUP_DEBUG, 1<<SNES_BTN_LT | 1<<SNES_BTN_RT, debug_stack);

    enable_combo_group(COMBO_GROUP_MAIN);
}

void start_task() {
    TaskHandle_t xHandle;
    void *param = NULL;
    BaseType_t taskCreated;

    ESP_LOGI(TAG, "creating SNES task with %d bytes of memory", SNES_TASK_STACK_SIZE);
    taskCreated = xTaskCreate(task_read_input, "SNES_INPUT", SNES_TASK_STACK_SIZE, param, tskIDLE_PRIORITY, &xHandle);
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "Failed to create snes task");
    }
}

void snes_init()
{
    gpio_init();
    combo_init();
    start_task();
}