#include "display_lvgl.h"
#include "hub75_driver.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"

static const char *TAG = "display_lvgl";

static SemaphoreHandle_t s_lvgl_mux;
static lv_display_t *s_disp;
static TaskHandle_t s_lvgl_task;

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    hub75_driver_draw_pixels_rgb565((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)w, (uint16_t)h, px_map);
    lv_display_flush_ready(disp);
}

static void lvgl_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(5);
}

static void lvgl_task_fn(void *arg)
{
    (void)arg;
    while (true) {
        display_lvgl_lock();
        lv_timer_handler();
        display_lvgl_unlock();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

esp_err_t display_lvgl_init(void)
{
    if (s_disp != NULL) {
        return ESP_OK;
    }

    s_lvgl_mux = xSemaphoreCreateRecursiveMutex();
    if (s_lvgl_mux == NULL) {
        return ESP_ERR_NO_MEM;
    }

    lv_init();

    s_disp = lv_display_create(hub75_driver_width(), hub75_driver_height());
    if (s_disp == NULL) {
        return ESP_ERR_NO_MEM;
    }

    static lv_color_t buf1[128 * 16];
    lv_display_set_buffers(s_disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, lvgl_flush_cb);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 5000));

    BaseType_t ok = xTaskCreate(lvgl_task_fn, "lvgl", 4096, NULL, 4, &s_lvgl_task);
    if (ok != pdPASS) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "LVGL %dx%d ready", hub75_driver_width(), hub75_driver_height());
    return ESP_OK;
}

void display_lvgl_lock(void)
{
    if (s_lvgl_mux) {
        xSemaphoreTakeRecursive(s_lvgl_mux, portMAX_DELAY);
    }
}

void display_lvgl_unlock(void)
{
    if (s_lvgl_mux) {
        xSemaphoreGiveRecursive(s_lvgl_mux);
    }
}

lv_obj_t *display_lvgl_screen(void)
{
    return lv_display_get_screen_active(s_disp);
}
