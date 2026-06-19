#include "diponto_font.h"
#include "hub75_driver.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "boot — milestone DIPONTO (FM6126A)");

    ESP_ERROR_CHECK(hub75_driver_init());
    ESP_ERROR_CHECK(hub75_driver_start());
    ESP_ERROR_CHECK(hub75_driver_set_brightness(255));

    esp_err_t err = diponto_font_draw_centered("DIPONTO", 255, 255, 255);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "draw failed: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "running");
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
