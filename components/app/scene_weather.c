#include "scene_weather.h"
#include "display_lvgl.h"

#include "lvgl.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t *s_root;
static lv_obj_t *s_city;
static lv_obj_t *s_temp;
static lv_obj_t *s_cond;
static lv_obj_t *s_err;
static weather_view_t s_view;

static void refresh(const weather_view_t *view)
{
    if (s_root == NULL || view == NULL) {
        return;
    }
    lv_label_set_text(s_city, "Porto Alegre");
    if (view->error) {
        lv_label_set_text(s_temp, "");
        lv_label_set_text(s_cond, "");
        lv_obj_clear_flag(s_err, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_err, view->error_msg ? view->error_msg : "Sem clima");
        return;
    }
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);
    char buf[16];
    snprintf(buf, sizeof(buf), "%s%d°C", view->stale ? "~" : "", view->temp_c);
    lv_label_set_text(s_temp, buf);
    lv_label_set_text(s_cond, view->condition ? view->condition : "");
}

static void async_refresh(void *arg)
{
    (void)arg;
    refresh(&s_view);
}

esp_err_t scene_weather_create(void)
{
    display_lvgl_lock();
    s_root = lv_obj_create(display_lvgl_screen());
    lv_obj_set_size(s_root, 128, 64);
    lv_obj_set_style_bg_color(s_root, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_root, 0, 0);
    lv_obj_set_style_pad_all(s_root, 0, 0);
    lv_obj_center(s_root);
    lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);

    s_city = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_city, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_city, &lv_font_montserrat_12, 0);
    lv_obj_align(s_city, LV_ALIGN_TOP_MID, 0, 2);

    s_temp = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_temp, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_temp, &lv_font_montserrat_24, 0);
    lv_obj_align(s_temp, LV_ALIGN_CENTER, 0, 0);

    s_cond = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_cond, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_cond, &lv_font_montserrat_12, 0);
    lv_obj_align(s_cond, LV_ALIGN_BOTTOM_MID, 0, -2);

    s_err = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_err, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_err, &lv_font_montserrat_12, 0);
    lv_obj_align(s_err, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);

    display_lvgl_unlock();
    return ESP_OK;
}

void scene_weather_update(const weather_view_t *view)
{
    if (view == NULL) {
        return;
    }
    s_view = *view;
    display_lvgl_async(async_refresh, NULL);
}

void scene_weather_show(bool visible)
{
    display_lvgl_lock();
    if (visible) {
        lv_obj_clear_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    }
    display_lvgl_unlock();
}
