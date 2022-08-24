#include "snes.h"

static const char *TAG = "SNES";

static void latch_low(void *arg)
{
    gpio_set_level(PIN_SNES_LATCH, 0);
    snes_register = snes_read_controller();
}
static const esp_timer_create_args_t timer_args_latch_low = {
    .callback = &latch_low,
    .name = "snes-latch-low",
};
esp_timer_handle_t timer_latch_low;

static void pulse_latch(void *arg)
{
    gpio_set_level(PIN_SNES_LATCH, 1);
    /* The snes controller's 4021 shift register needs at least a 180ns pulse
     * width for 5v.
     *
     * See Pg. 4 of the datasheet:
     *   https://www.ti.com/lit/ds/symlink/cd4021b-q1.pdf?ts=1661218995201 */
    ESP_ERROR_CHECK(esp_timer_start_once(timer_latch_low, SNES_LATCH_PULSE_WIDTH));
}
static const esp_timer_create_args_t timer_args_pulse_latch = {
    .callback = &pulse_latch,
    .name = "snes-pulse-latch",
};
esp_timer_handle_t timer_pulse_latch;

void timer_init()
{
    ESP_ERROR_CHECK(esp_timer_create(
        &timer_args_latch_low,
        &timer_latch_low
    ));
    ESP_ERROR_CHECK(esp_timer_create(
        &timer_args_pulse_latch,
        &timer_pulse_latch
    ));
    ESP_LOGI(TAG, "latch pulse width configured to %dus resolution",
        SNES_LATCH_PULSE_WIDTH
    );
    ESP_LOGI(TAG, "starting periodic timer with %dus resolution",
        SNES_POLLILNG_RATE
    );

    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_pulse_latch, SNES_POLLILNG_RATE));
}

void snes_pulse_clock()
{
    // shift register to next bit to be read
    gpio_set_level(PIN_SNES_CLOCK, 1);
    gpio_set_level(PIN_SNES_CLOCK, 0);
}

void gpio_init()
{
    size_t num_gpio_confs = sizeof(io_confs) / sizeof(gpio_config_t);
    for (int i = 0; i < num_gpio_confs; ++i) {
        gpio_config(&io_confs[i]);
    }
    ESP_LOGI(TAG, "gpio ports initialized");
}

void snes_init()
{
    gpio_init();
    timer_init();
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

int snes_read_controller()
{
    // controller has to be sent a high/low latch before reading
    int new_register = SNES_REGISTER_DEFAULT;
    for (uint8_t i = 0; i < SNES_REGISTER_NUM_BITS; ++i) {
        if (i != 0) {
            snes_pulse_clock();
        }
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