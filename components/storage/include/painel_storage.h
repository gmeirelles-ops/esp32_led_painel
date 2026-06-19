#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char tz[32];
    uint32_t dwell_ms;
} painel_config_t;

typedef struct {
    char ssid[33];
    char pass[65];
} painel_wifi_creds_t;

typedef struct {
    int16_t temp_x10;
    int16_t weather_code;
    int64_t fetched_unix;
    bool valid;
} painel_weather_cache_t;

esp_err_t painel_storage_init(void);
esp_err_t painel_storage_load_config(painel_config_t *out);
esp_err_t painel_storage_save_config(const painel_config_t *cfg);
esp_err_t painel_storage_load_wifi(painel_wifi_creds_t *out);
esp_err_t painel_storage_load_weather_cache(painel_weather_cache_t *out);
esp_err_t painel_storage_save_weather_cache(const painel_weather_cache_t *cache);
void painel_tz_apply(const char *tz);

#ifdef __cplusplus
}
#endif
