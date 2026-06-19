#include "hub75_driver.h"
#include "hub75_board_config.h"

#include "esp_log.h"
#include "hub75.h"

#include <new>

static const char *TAG = "hub75_drv";
static Hub75Driver *s_driver = nullptr;

extern "C" esp_err_t hub75_driver_init(void)
{
    if (s_driver != nullptr) {
        return ESP_OK;
    }

    Hub75Config config = hub75_get_matrixportal_config();
    s_driver = new (std::nothrow) Hub75Driver(config);
    if (s_driver == nullptr) {
        return ESP_ERR_NO_MEM;
    }
    if (!s_driver->begin()) {
        ESP_LOGE(TAG, "hub75 begin failed");
        delete s_driver;
        s_driver = nullptr;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "display %ux%u", s_driver->get_width(), s_driver->get_height());
    return ESP_OK;
}

extern "C" esp_err_t hub75_driver_start(void)
{
    return s_driver != nullptr ? ESP_OK : ESP_ERR_INVALID_STATE;
}

extern "C" int hub75_driver_width(void)
{
    return s_driver ? s_driver->get_width() : 0;
}

extern "C" int hub75_driver_height(void)
{
    return s_driver ? s_driver->get_height() : 0;
}

extern "C" esp_err_t hub75_driver_set_brightness(uint8_t level)
{
    if (s_driver == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    s_driver->set_brightness(level);
    return ESP_OK;
}

extern "C" esp_err_t hub75_driver_fill_rgb(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g,
                                           uint8_t b)
{
    if (s_driver == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    s_driver->fill(x, y, w, h, r, g, b);
    return ESP_OK;
}

extern "C" esp_err_t hub75_driver_draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (s_driver == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    s_driver->set_pixel(x, y, r, g, b);
    return ESP_OK;
}

extern "C" esp_err_t hub75_driver_draw_pixels_rgb565(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                                     const uint8_t *buffer)
{
    if (s_driver == nullptr || buffer == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }
    s_driver->draw_pixels(x, y, w, h, buffer, Hub75PixelFormat::RGB565, Hub75ColorOrder::RGB, false);
    return ESP_OK;
}
