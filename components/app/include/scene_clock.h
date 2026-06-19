#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CLOCK_SYNC_PENDING = 0,
    CLOCK_SYNC_SYNCED,
    CLOCK_SYNC_ERROR,
} clock_sync_status_t;

esp_err_t scene_clock_create(void);
void scene_clock_set_wifi(bool up);
void scene_clock_set_sync(clock_sync_status_t st);
void scene_clock_show(bool visible);

#ifdef __cplusplus
}
#endif
