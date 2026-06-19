#include "scene_clock.h"
#include "display_lvgl.h"

#include "esp_timer.h"
#include "lvgl.h"
#include <stdio.h>
#include <time.h>

static lv_obj_t *s_root;
static lv_obj_t *s_time;
static lv_obj_t *s_date;
static lv_obj_t *s_err;
static clock_sync_status_t s_sync = CLOCK_SYNC_PENDING;
static bool s_wifi;
static esp_timer_handle_t s_tick;

static void refresh(void)
{
    if (s_time == NULL) {
        return;
    }
    if (!s_wifi) {
        lv_label_set_text(s_time, "");
        lv_label_set_text(s_date, "");
        lv_obj_clear_flag(s_err, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_err, "Sem Wi-Fi");
        return;
    }
    if (s_sync != CLOCK_SYNC_SYNCED) {
        lv_label_set_text(s_time, "");
        lv_label_set_text(s_date, "");
        lv_obj_clear_flag(s_err, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_err, "Sem hora");
        return;
    }
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", tm_info.tm_hour, tm_info.tm_min);
    lv_label_set_text(s_time, buf);
    snprintf(buf, sizeof(buf), "%02d/%02d", tm_info.tm_mday, tm_info.tm_mon + 1);
    lv_label_set_text(s_date, buf);
}

static void tick_cb(void *arg)
{
    (void)arg;
    display_lvgl_lock();
    refresh();
    display_lvgl_unlock();
}

esp_err_t scene_clock_create(void)
{
    display_lvgl_lock();
    s_root = lv_obj_create(display_lvgl_screen());
    lv_obj_set_size(s_root, 128, 64);
    lv_obj_set_style_bg_color(s_root, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_root, 0, 0);
    lv_obj_set_style_pad_all(s_root, 0, 0);
    lv_obj_center(s_root);

    s_time = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_time, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_time, &lv_font_montserrat_24, 0);
    lv_obj_align(s_time, LV_ALIGN_CENTER, 0, -6);

    s_date = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_date, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_date, &lv_font_montserrat_12, 0);
    lv_obj_align(s_date, LV_ALIGN_BOTTOM_MID, 0, -2);

    s_err = lv_label_create(s_root);
    lv_obj_set_style_text_color(s_err, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_err, &lv_font_montserrat_12, 0);
    lv_obj_align(s_err, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);

    display_lvgl_unlock();

    const esp_timer_create_args_t args = {.callback = tick_cb, .name = "clock_tick"};
    ESP_ERROR_CHECK(esp_timer_create(&args, &s_tick));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_tick, 1000000));
    return ESP_OK;
}

void scene_clock_set_wifi(bool up)
{
    s_wifi = up;
    display_lvgl_lock();
    refresh();
    display_lvgl_unlock();
}

void scene_clock_set_sync(clock_sync_status_t st)
{
    s_sync = st;
    display_lvgl_lock();
    refresh();
    display_lvgl_unlock();
}

void scene_clock_show(bool visible)
{
    display_lvgl_lock();
    if (visible) {
        lv_obj_clear_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    }
    display_lvgl_unlock();
}
