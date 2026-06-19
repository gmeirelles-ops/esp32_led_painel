#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int temp_c;
    int weather_code;
    bool valid;
} open_meteo_current_t;

esp_err_t open_meteo_fetch_current(float lat, float lon, const char *tz, open_meteo_current_t *out);
const char *wmo_condition_pt(int code);

#ifdef __cplusplus
}
#endif
