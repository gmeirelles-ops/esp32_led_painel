#include "app_controller.h"

#include "connectivity.h"
#include "dashboard_state.h"
#include "display_lvgl.h"
#include "open_meteo.h"
#include "painel_storage.h"
#include "scene_clock.h"
#include "scene_weather.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "sdkconfig.h"
#include <time.h>

static const char *TAG = "app_ctrl";
static const int WEATHER_INTERVAL_S = 1800;

static dashboard_state_t s_dash;
static painel_config_t s_cfg;
static weather_view_t s_weather;
static lv_obj_t *s_splash;
static esp_timer_handle_t s_dwell_timer;
static esp_timer_handle_t s_weather_timer;

static void apply_scene(dash_scene_t scene)
{
    scene_clock_show(scene == DASH_SCENE_CLOCK);
    scene_weather_show(scene == DASH_SCENE_WEATHER);
}

static void dwell_cb(void *arg)
{
    (void)arg;
    if (!s_dash.rotation_enabled) {
        return;
    }
    dashboard_state_toggle(&s_dash);
    display_lvgl_lock();
    apply_scene(s_dash.active);
    display_lvgl_unlock();
}

static void update_weather_view(bool live_ok, const open_meteo_current_t *live, const painel_weather_cache_t *cache)
{
    if (!wifi_manager_is_connected()) {
        s_weather.error = true;
        s_weather.error_msg = "Sem Wi-Fi";
        scene_weather_update(&s_weather);
        return;
    }
    if (live_ok && live) {
        s_weather.error = false;
        s_weather.temp_c = live->temp_c;
        s_weather.condition = wmo_condition_pt(live->weather_code);
        s_weather.stale = false;
    } else if (cache && cache->valid) {
        s_weather.error = false;
        s_weather.temp_c = cache->temp_x10 / 10;
        s_weather.condition = wmo_condition_pt(cache->weather_code);
        time_t now = time(NULL);
        s_weather.stale = (now - cache->fetched_unix) > WEATHER_INTERVAL_S;
    } else {
        s_weather.error = true;
        s_weather.error_msg = "Sem clima";
    }
    scene_weather_update(&s_weather);
}

static void fetch_weather(void)
{
    open_meteo_current_t live = {0};
    esp_err_t err = open_meteo_fetch_current(CONFIG_PAINEL_POA_LAT, CONFIG_PAINEL_POA_LON, s_cfg.tz, &live);
    painel_weather_cache_t cache = {0};
    if (err == ESP_OK && live.valid) {
        cache.valid = true;
        cache.temp_x10 = (int16_t)(live.temp_c * 10);
        cache.weather_code = (int16_t)live.weather_code;
        cache.fetched_unix = time(NULL);
        painel_storage_save_weather_cache(&cache);
        update_weather_view(true, &live, NULL);
    } else {
        painel_storage_load_weather_cache(&cache);
        update_weather_view(false, NULL, &cache);
    }
}

static void weather_timer_cb(void *arg)
{
    (void)arg;
    if (wifi_manager_is_connected()) {
        fetch_weather();
    }
}

static void on_connectivity(app_connectivity_event_t evt, void *ctx)
{
    (void)ctx;
    switch (evt) {
    case APP_EVT_WIFI_UP:
        scene_clock_set_wifi(true);
        sntp_service_start(s_cfg.tz);
        fetch_weather();
        break;
    case APP_EVT_WIFI_DOWN:
        scene_clock_set_wifi(false);
        scene_clock_set_sync(CLOCK_SYNC_ERROR);
        update_weather_view(false, NULL, NULL);
        break;
    case APP_EVT_SNTP_SYNCED:
        scene_clock_set_sync(CLOCK_SYNC_SYNCED);
        break;
    case APP_EVT_SNTP_FAIL:
        scene_clock_set_sync(CLOCK_SYNC_ERROR);
        break;
    default:
        break;
    }
}

static void finish_init(void)
{
    display_lvgl_lock();
    if (s_splash) {
        lv_obj_del(s_splash);
        s_splash = NULL;
    }
    s_dash.init_complete = true;
    s_dash.rotation_enabled = true;
    dashboard_state_set_active(&s_dash, DASH_SCENE_CLOCK);
    apply_scene(DASH_SCENE_CLOCK);
    display_lvgl_unlock();

    const esp_timer_create_args_t dwell = {.callback = dwell_cb, .name = "dwell"};
    ESP_ERROR_CHECK(esp_timer_create(&dwell, &s_dwell_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_dwell_timer, s_dash.dwell_ms * 1000ULL));

    const esp_timer_create_args_t wx = {.callback = weather_timer_cb, .name = "weather"};
    ESP_ERROR_CHECK(esp_timer_create(&wx, &s_weather_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_weather_timer, WEATHER_INTERVAL_S * 1000000ULL));
}

static void app_task(void *arg)
{
    (void)arg;
    painel_weather_cache_t cache = {0};
    painel_storage_load_weather_cache(&cache);
    if (cache.valid) {
        update_weather_view(false, NULL, &cache);
    }
    finish_init();
    vTaskDelete(NULL);
}

esp_err_t app_controller_start(void)
{
    ESP_ERROR_CHECK(painel_storage_load_config(&s_cfg));
    dashboard_state_init(&s_dash, s_cfg.dwell_ms);

    display_lvgl_lock();
    s_splash = lv_label_create(display_lvgl_screen());
    lv_label_set_text(s_splash, "Iniciando...");
    lv_obj_set_style_text_color(s_splash, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_splash, &lv_font_montserrat_12, 0);
    lv_obj_center(s_splash);
    display_lvgl_unlock();

    ESP_ERROR_CHECK(scene_clock_create());
    ESP_ERROR_CHECK(scene_weather_create());

    wifi_manager_set_callback(on_connectivity, NULL);
    scene_clock_set_wifi(wifi_manager_is_connected());

    xTaskCreate(app_task, "app", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "started dwell=%ums", (unsigned)s_dash.dwell_ms);
    return ESP_OK;
}
