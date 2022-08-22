#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define WIFI_PASSWORD  CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAX_RETRY CONFIG_ESP_WIFI_MAX_RETRY

static EventGroupHandle_t wifi_event_group;
static StaticEventGroup_t _static_wifi_event_group;
static int connect_retry_num = 0;

void wifi_init();