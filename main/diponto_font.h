#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Draw ASCII string centered on the HUB75 canvas (5×7 font, scale 2). */
esp_err_t diponto_font_draw_centered(const char *text, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
