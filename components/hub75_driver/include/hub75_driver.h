#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hub75_driver_init(void);
esp_err_t hub75_driver_start(void);
int hub75_driver_width(void);
int hub75_driver_height(void);
esp_err_t hub75_driver_set_brightness(uint8_t level);
esp_err_t hub75_driver_fill_rgb(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);
esp_err_t hub75_driver_draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
