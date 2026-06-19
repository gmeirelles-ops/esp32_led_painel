#pragma once

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t display_lvgl_init(void);
void display_lvgl_lock(void);
void display_lvgl_unlock(void);
lv_obj_t *display_lvgl_screen(void);
void display_lvgl_async(void (*fn)(void *), void *user_data);
int display_lvgl_hor_res(void);
int display_lvgl_ver_res(void);

#ifdef __cplusplus
}
#endif
