#pragma once

#include "esp_err.h"
#include <stdbool.h>

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int temp_c;
    const char *condition;
    bool stale;
    bool error;
    const char *error_msg;
} weather_view_t;

esp_err_t scene_weather_create(void);
void scene_weather_update(const weather_view_t *view);
void scene_weather_show(bool visible);
lv_obj_t *scene_weather_root(void);

#ifdef __cplusplus
}
#endif
