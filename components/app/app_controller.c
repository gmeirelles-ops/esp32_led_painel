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
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "sdkconfig.h"
#include <time.h>

static const char *TAG = "app_ctrl";
static const int WEATHER_INTERVAL_S = 1800;

typedef enum {
    APP_Q_WIFI_UP = 0,
    APP_Q_WIFI_DOWN,
    APP_Q_SNTP_SYNCED,
    APP_Q_SNTP_FAIL,
    APP_Q_FETCH_WEATHER,
} app_queue_evt_t;

static dashboard_state_t s_dash;
static painel_config_t s_cfg;
static weather_view_t s_weather;
static lv_obj_t *s_splash;
static lv_timer_t *s_dwell_timer;
static esp_timer_handle_t s_weather_timer;
static QueueHandle_t s_app_queue;
static TaskHandle_t s_net_task;

static void apply_scene(dash_scene_t scene)
{
    scene_clock_show(scene == DASH_SCENE_CLOCK);
    scene_weather_show(scene == DASH_SCENE_WEATHER);
}

static void dwell_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!s_dash.rotation_enabled) {
        return;
    }
    dashboard_state_toggle(&s_dash);
    apply_scene(s_dash.active);
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

static void post_evt(app_queue_evt_t evt)
{
    if (s_app_queue != NULL) {
        xQueueSend(s_app_queue, &evt, 0);
    }
}

static void weather_timer_cb(void *arg)
{
    (void)arg;
    post_evt(APP_Q_FETCH_WEATHER);
}

static void on_connectivity(app_connectivity_event_t evt, void *ctx)
{
    (void)ctx;
    switch (evt) {
    case APP_EVT_WIFI_UP:
        post_evt(APP_Q_WIFI_UP);
        break;
    case APP_EVT_WIFI_DOWN:
        post_evt(APP_Q_WIFI_DOWN);
        break;
    case APP_EVT_SNTP_SYNCED:
        post_evt(APP_Q_SNTP_SYNCED);
        break;
    case APP_EVT_SNTP_FAIL:
        post_evt(APP_Q_SNTP_FAIL);
        break;
    default:
        break;
    }
}

static void net_worker(void *arg)
{
    (void)arg;
    app_queue_evt_t evt;
    for (;;) {
        if (xQueueReceive(s_app_queue, &evt, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        switch (evt) {
        case APP_Q_WIFI_UP:
            scene_clock_set_wifi(true);
            sntp_service_start(s_cfg.tz);
            fetch_weather();
            break;
        case APP_Q_WIFI_DOWN:
            scene_clock_set_wifi(false);
            scene_clock_set_sync(CLOCK_SYNC_ERROR);
            update_weather_view(false, NULL, NULL);
            break;
        case APP_Q_SNTP_SYNCED:
            scene_clock_set_sync(CLOCK_SYNC_SYNCED);
            break;
        case APP_Q_SNTP_FAIL:
            scene_clock_set_sync(CLOCK_SYNC_ERROR);
            break;
        case APP_Q_FETCH_WEATHER:
            if (wifi_manager_is_connected()) {
                fetch_weather();
            }
            break;
        default:
            break;
        }
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

    display_lvgl_lock();
    s_dwell_timer = lv_timer_create(dwell_cb, s_dash.dwell_ms, NULL);
    display_lvgl_unlock();

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

    s_app_queue = xQueueCreate(8, sizeof(app_queue_evt_t));
    if (s_app_queue == NULL) {
        return ESP_ERR_NO_MEM;
    }
    BaseType_t ok = xTaskCreate(net_worker, "net_worker", 16384, NULL, 5, &s_net_task);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

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
