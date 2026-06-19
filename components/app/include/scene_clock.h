#pragma once

#include "esp_err.h"
#include <stdbool.h>

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

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
lv_obj_t *scene_clock_root(void);

#ifdef __cplusplus
}
#endif
