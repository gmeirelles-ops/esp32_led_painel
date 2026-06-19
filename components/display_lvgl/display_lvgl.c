#include "display_lvgl.h"
#include "hub75_driver.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"

static const char *TAG = "display_lvgl";

static SemaphoreHandle_t s_lvgl_mux;
static lv_display_t *s_disp;
static TaskHandle_t s_lvgl_task;
static lv_color_t *s_fb;
static size_t s_fb_bytes;

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
    esp_task_wdt_add(NULL);
    while (true) {
        display_lvgl_lock();
        uint32_t delay_ms = lv_timer_handler();
        display_lvgl_unlock();
        esp_task_wdt_reset();
        if (delay_ms < 5) {
            delay_ms = 5;
        } else if (delay_ms > 500) {
            delay_ms = 500;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

esp_err_t display_lvgl_init(void)
{
    if (s_disp != NULL) {
        return ESP_OK;
    }

    const int hor = hub75_driver_width();
    const int ver = hub75_driver_height();
    if (hor <= 0 || ver <= 0) {
        ESP_LOGE(TAG, "invalid display size %dx%d", hor, ver);
        return ESP_ERR_INVALID_STATE;
    }

    s_lvgl_mux = xSemaphoreCreateRecursiveMutex();
    if (s_lvgl_mux == NULL) {
        return ESP_ERR_NO_MEM;
    }

    s_fb_bytes = (size_t)hor * (size_t)ver * sizeof(lv_color_t);
    s_fb = (lv_color_t *)heap_caps_malloc(s_fb_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (s_fb == NULL) {
        ESP_LOGE(TAG, "LVGL framebuffer alloc failed (%u bytes)", (unsigned)s_fb_bytes);
        vSemaphoreDelete(s_lvgl_mux);
        s_lvgl_mux = NULL;
        return ESP_ERR_NO_MEM;
    }

    lv_init();

    s_disp = lv_display_create(hor, ver);
    if (s_disp == NULL) {
        heap_caps_free(s_fb);
        s_fb = NULL;
        vSemaphoreDelete(s_lvgl_mux);
        s_lvgl_mux = NULL;
        return ESP_ERR_NO_MEM;
    }

    lv_display_set_buffers(s_disp, s_fb, NULL, s_fb_bytes, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(s_disp, lvgl_flush_cb);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);

    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 5000));

    BaseType_t ok = xTaskCreatePinnedToCore(lvgl_task_fn, "lvgl", 8192, NULL, 4, &s_lvgl_task, 1);
    if (ok != pdPASS) {
        heap_caps_free(s_fb);
        s_fb = NULL;
        vSemaphoreDelete(s_lvgl_mux);
        s_lvgl_mux = NULL;
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "LVGL %dx%d ready (full frame, core 1, fb %u bytes, free heap %lu)", hor, ver,
             (unsigned)s_fb_bytes, (unsigned long)esp_get_free_heap_size());
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

void display_lvgl_async(void (*fn)(void *), void *user_data)
{
    lv_async_call(fn, user_data);
}

int display_lvgl_hor_res(void)
{
    return s_disp != NULL ? (int)lv_display_get_horizontal_resolution(s_disp) : hub75_driver_width();
}

int display_lvgl_ver_res(void)
{
    return s_disp != NULL ? (int)lv_display_get_vertical_resolution(s_disp) : hub75_driver_height();
}
