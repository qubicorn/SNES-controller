#pragma once
#include <sys/param.h>

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_tls.h"

void http_post_kv(char *k, int v);