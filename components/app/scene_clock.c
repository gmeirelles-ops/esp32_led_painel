#include "scene_clock.h"
#include "display_lvgl.h"
#include "scene_layout.h"

#include "esp_log.h"
#include "lvgl.h"
#include <stdio.h>
#include <time.h>

static const char *TAG = "scene_clock";

static const char *const DOW_PT[] = {"DOMINGO", "SEGUNDA", "TERCA", "QUARTA", "QUINTA", "SEXTA", "SABADO"};
static const char *const DOW_SHORT[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};

static lv_obj_t *s_root;
static lv_obj_t *s_hero;
static lv_obj_t *s_footer;
static lv_obj_t *s_hour;
static lv_obj_t *s_colon;
static lv_obj_t *s_min;
static lv_obj_t *s_weekday;
static lv_obj_t *s_date;
static lv_obj_t *s_err;
static clock_sync_status_t s_sync = CLOCK_SYNC_PENDING;
static bool s_wifi;
static bool s_colon_on = true;
static lv_timer_t *s_tick;
static lv_timer_t *s_blink;
static scene_layout_profile_t s_layout;

static const char *dow_label(int wday)
{
    return s_layout.tier == SCENE_LAYOUT_LARGE ? DOW_PT[wday] : DOW_SHORT[wday];
}

static void set_live_visible(bool on)
{
    if (on) {
        lv_obj_clear_flag(s_hero, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_footer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_hero, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_footer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_err, LV_OBJ_FLAG_HIDDEN);
    }
}

static void show_error(const char *msg)
{
    set_live_visible(false);
    lv_label_set_text(s_err, msg);
}

static void refresh(void)
{
    if (s_hour == NULL) {
        return;
    }
    if (!s_wifi) {
        show_error("SEM WIFI");
        return;
    }
    if (s_sync == CLOCK_SYNC_PENDING) {
        show_error("SYNC...");
        return;
    }
    if (s_sync != CLOCK_SYNC_SYNCED) {
        show_error("SEM HORA");
        return;
    }

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    if (tm_info.tm_year + 1900 <= 2020) {
        ESP_LOGW(TAG, "synced but invalid tm (year=%d)", tm_info.tm_year + 1900);
        show_error("SYNC...");
        return;
    }

    set_live_visible(true);
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d", tm_info.tm_hour);
    lv_label_set_text(s_hour, buf);
    snprintf(buf, sizeof(buf), "%02d", tm_info.tm_min);
    lv_label_set_text(s_min, buf);
    lv_obj_set_style_opa(s_colon, s_colon_on ? LV_OPA_COVER : LV_OPA_20, 0);

    char date_buf[16];
    snprintf(date_buf, sizeof(date_buf), "%02d/%02d", tm_info.tm_mday, tm_info.tm_mon + 1);
    lv_label_set_text(s_date, date_buf);
    lv_label_set_text(s_weekday, dow_label(tm_info.tm_wday));
}

static void tick_cb(lv_timer_t *timer)
{
    (void)timer;
    refresh();
}

static void blink_cb(lv_timer_t *timer)
{
    (void)timer;
    s_colon_on = !s_colon_on;
    if (s_sync == CLOCK_SYNC_SYNCED && s_wifi && s_colon != NULL) {
        lv_obj_set_style_opa(s_colon, s_colon_on ? LV_OPA_COVER : LV_OPA_20, 0);
    }
}

static void async_refresh(void *arg)
{
    (void)arg;
    refresh();
}

esp_err_t scene_clock_create(void)
{
    display_lvgl_lock();
    s_layout = scene_layout_get();

    s_root = lv_obj_create(display_lvgl_screen());
    lv_obj_set_size(s_root, s_layout.hor, s_layout.ver);
    lv_obj_center(s_root);
    lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    scene_layout_init_root(s_root, &s_layout);

    s_hero = scene_layout_panel(s_root, 0, 0, s_layout.hor, s_layout.clock_hero_h, s_layout.color_clock_hero);

    s_hour = lv_label_create(s_hero);
    lv_obj_set_style_text_font(s_hour, s_layout.font_hero, 0);
    lv_obj_set_style_text_color(s_hour, s_layout.color_on_hero, 0);
    lv_obj_align(s_hour, LV_ALIGN_LEFT_MID, s_layout.tier == SCENE_LAYOUT_LARGE ? 16 : 8, 0);

    s_colon = lv_label_create(s_hero);
    lv_label_set_text(s_colon, ":");
    lv_obj_set_style_text_font(s_colon, s_layout.font_hero, 0);
    lv_obj_set_style_text_color(s_colon, s_layout.color_on_hero, 0);
    lv_obj_align(s_colon, LV_ALIGN_CENTER, 0, 0);

    s_min = lv_label_create(s_hero);
    lv_obj_set_style_text_font(s_min, s_layout.font_hero, 0);
    lv_obj_set_style_text_color(s_min, s_layout.color_on_hero, 0);
    lv_obj_align(s_min, LV_ALIGN_RIGHT_MID, s_layout.tier == SCENE_LAYOUT_LARGE ? -16 : -8, 0);

    s_footer = scene_layout_panel(s_root, 0, s_layout.clock_hero_h, s_layout.hor,
                                  s_layout.ver - s_layout.clock_hero_h, s_layout.color_panel);

    s_weekday = lv_label_create(s_footer);
    lv_obj_set_style_text_font(s_weekday, s_layout.font_body, 0);
    lv_obj_set_style_text_color(s_weekday, s_layout.color_text, 0);
    lv_obj_align(s_weekday, LV_ALIGN_LEFT_MID, 6, 0);

    s_date = lv_label_create(s_footer);
    lv_obj_set_style_text_font(s_date, s_layout.font_caption, 0);
    lv_obj_set_style_text_color(s_date, s_layout.color_muted, 0);
    lv_obj_align(s_date, LV_ALIGN_RIGHT_MID, -6, 0);

    s_err = lv_label_create(s_root);
    lv_obj_set_style_text_font(s_err, s_layout.font_body, 0);
    lv_obj_set_style_text_color(s_err, s_layout.color_text, 0);
    lv_obj_align(s_err, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(s_err, LV_OBJ_FLAG_HIDDEN);

    show_error("SYNC...");

    s_tick = lv_timer_create(tick_cb, 1000, NULL);
    s_blink = lv_timer_create(blink_cb, 500, NULL);
    display_lvgl_unlock();
    return ESP_OK;
}

void scene_clock_set_wifi(bool up)
{
    s_wifi = up;
    display_lvgl_async(async_refresh, NULL);
}

void scene_clock_set_sync(clock_sync_status_t st)
{
    s_sync = st;
    display_lvgl_async(async_refresh, NULL);
}

void scene_clock_show(bool visible)
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

lv_obj_t *scene_clock_root(void)
{
    return s_root;
}
