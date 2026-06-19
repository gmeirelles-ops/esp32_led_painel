#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_EVT_WIFI_UP = 0,
    APP_EVT_WIFI_DOWN,
    APP_EVT_SNTP_SYNCED,
    APP_EVT_SNTP_FAIL,
} app_connectivity_event_t;

typedef void (*app_connectivity_cb_t)(app_connectivity_event_t evt, void *ctx);

esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_start(void);
bool wifi_manager_is_connected(void);
void wifi_manager_set_callback(app_connectivity_cb_t cb, void *ctx);
esp_err_t sntp_service_start(const char *tz);
bool sntp_service_is_synced(void);
esp_err_t http_get_string(const char *url, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
