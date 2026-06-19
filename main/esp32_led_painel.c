#include "app_controller.h"
#include "connectivity.h"
#include "display_lvgl.h"
#include "hub75_driver.h"
#include "painel_storage.h"

#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "boot — LVGL POA dashboard");

    ESP_ERROR_CHECK(painel_storage_init());
    ESP_ERROR_CHECK(hub75_driver_init());
    ESP_ERROR_CHECK(hub75_driver_start());
    ESP_ERROR_CHECK(hub75_driver_set_brightness(50));

    ESP_ERROR_CHECK(display_lvgl_init());
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_start());
    ESP_ERROR_CHECK(app_controller_start());

    ESP_LOGI(TAG, "running");
}
