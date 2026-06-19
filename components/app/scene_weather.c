#include "scene_weather.h"
#include "display_lvgl.h"
#include "scene_layout.h"

#include "lvgl.h"
#include <stdio.h>
#include <string.h>

static lv_obj_t *s_root;
static lv_obj_t *s_body;
static lv_obj_t *s_temp;
static lv_obj_t *s_cond;
static lv_obj_t *s_city;
static lv_obj_t *s_stale;
static lv_obj_t *s_err;
static weather_view_t s_view;
static scene_layout_profile_t s_layout;

static void show_error(const char *msg)
{
    lv_obj_add_flag(s_body, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_err, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(s_err, msg);
}

static void show_weather_content(void)
{
    lv_obj_clear_flag(s_body, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);
}

static void refresh(const weather_view_t *view)
{
    if (s_root == NULL || view == NULL) {
        return;
    }
    if (view->error) {
        show_error(view->error_msg ? view->error_msg : "SEM CLIMA");
        return;
    }

    show_weather_content();
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", view->temp_c);
    lv_label_set_text(s_temp, buf);
    lv_label_set_text(s_cond, view->condition ? view->condition : "");
    lv_obj_set_style_opa(s_temp, view->stale ? LV_OPA_60 : LV_OPA_COVER, 0);
    lv_obj_set_style_opa(s_cond, view->stale ? LV_OPA_60 : LV_OPA_COVER, 0);

    if (view->stale) {
        lv_obj_clear_flag(s_stale, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_stale, LV_OBJ_FLAG_HIDDEN);
    }
}

static void async_refresh(void *arg)
{
    (void)arg;
    refresh(&s_view);
}

esp_err_t scene_weather_create(void)
{
    display_lvgl_lock();
    s_layout = scene_layout_get();

    s_root = lv_obj_create(display_lvgl_screen());
    lv_obj_set_size(s_root, s_layout.hor, s_layout.ver);
    lv_obj_center(s_root);
    lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    scene_layout_init_root(s_root, &s_layout);

    scene_layout_panel(s_root, 0, 0, s_layout.weather_band_w, s_layout.ver, s_layout.color_weather_band);

    s_body = scene_layout_panel(s_root, s_layout.weather_band_w, 0,
                                s_layout.hor - s_layout.weather_band_w, s_layout.ver, s_layout.color_panel);

    s_temp = lv_label_create(s_body);
    lv_obj_set_style_text_font(s_temp, s_layout.font_hero, 0);
    lv_obj_set_style_text_color(s_temp, s_layout.color_weather_band, 0);
    lv_obj_align(s_temp, LV_ALIGN_TOP_LEFT, 4, s_layout.tier == SCENE_LAYOUT_LARGE ? 6 : 2);

    lv_obj_t *unit = lv_label_create(s_body);
    lv_label_set_text(unit, "C");
    lv_obj_set_style_text_font(unit, s_layout.font_body, 0);
    lv_obj_set_style_text_color(unit, s_layout.color_muted, 0);
    lv_obj_align_to(unit, s_temp, LV_ALIGN_OUT_RIGHT_TOP, 2, s_layout.tier == SCENE_LAYOUT_LARGE ? 8 : 4);

    s_cond = lv_label_create(s_body);
    lv_obj_set_style_text_font(s_cond, s_layout.font_body, 0);
    lv_obj_set_style_text_color(s_cond, s_layout.color_text, 0);
    lv_obj_set_width(s_cond, lv_obj_get_width(s_body) - 8);
    lv_label_set_long_mode(s_cond, LV_LABEL_LONG_DOT);
    lv_obj_align(s_cond, LV_ALIGN_LEFT_MID, 4, s_layout.tier == SCENE_LAYOUT_LARGE ? 8 : 4);

    s_city = lv_label_create(s_body);
    lv_label_set_text(s_city, "POA");
    lv_obj_set_style_text_font(s_city, s_layout.font_caption, 0);
    lv_obj_set_style_text_color(s_city, s_layout.color_muted, 0);
    lv_obj_align(s_city, LV_ALIGN_BOTTOM_RIGHT, -4, -2);

    s_stale = lv_label_create(s_body);
    lv_label_set_text(s_stale, "~");
    lv_obj_set_style_text_font(s_stale, s_layout.font_caption, 0);
    lv_obj_set_style_text_color(s_stale, s_layout.color_muted, 0);
    lv_obj_align(s_stale, LV_ALIGN_TOP_RIGHT, -4, 2);
    lv_obj_add_flag(s_stale, LV_OBJ_FLAG_HIDDEN);

    s_err = lv_label_create(s_root);
    lv_obj_set_style_text_font(s_err, s_layout.font_body, 0);
    lv_obj_set_style_text_color(s_err, s_layout.color_text, 0);
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
        lv_obj_set_style_opa(s_root, LV_OPA_COVER, 0);
    } else {
        lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    }
    display_lvgl_unlock();
}

lv_obj_t *scene_weather_root(void)
{
    return s_root;
}
