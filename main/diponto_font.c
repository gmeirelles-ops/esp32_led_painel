#include "diponto_font.h"

#include "hub75_driver.h"

#include "esp_log.h"
#include <ctype.h>
#include <string.h>

static const char *TAG = "diponto_font";

#define FONT_W 5
#define FONT_H 7
#define FONT_SCALE 2
#define CHAR_SPACING 1

/* 5×7 uppercase A–Z (columns left-to-right, LSB = top row). */
static const uint8_t font5x7[26][5] = {
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, /* G */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x26, 0x49, 0x49, 0x49, 0x32}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x03, 0x04, 0x78, 0x04, 0x03}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
};

static const uint8_t *glyph_for(char c)
{
    if (c >= 'a' && c <= 'z') {
        c = (char)toupper((unsigned char)c);
    }
    if (c < 'A' || c > 'Z') {
        return NULL;
    }
    return font5x7[c - 'A'];
}

static int text_width_px(const char *text)
{
    size_t len = strlen(text);
    if (len == 0) {
        return 0;
    }
    return (int)(len * (FONT_W + CHAR_SPACING) * FONT_SCALE - CHAR_SPACING * FONT_SCALE);
}

static void draw_glyph(int x0, int y0, const uint8_t *glyph, uint8_t r, uint8_t g, uint8_t b)
{
    for (int col = 0; col < FONT_W; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < FONT_H; row++) {
            if ((bits >> row) & 1) {
                for (int sy = 0; sy < FONT_SCALE; sy++) {
                    for (int sx = 0; sx < FONT_SCALE; sx++) {
                        hub75_driver_draw_pixel((uint16_t)(x0 + col * FONT_SCALE + sx),
                                                (uint16_t)(y0 + row * FONT_SCALE + sy), r, g, b);
                    }
                }
            }
        }
    }
}

esp_err_t diponto_font_draw_centered(const char *text, uint8_t r, uint8_t g, uint8_t b)
{
    if (text == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const int panel_w = hub75_driver_width();
    const int panel_h = hub75_driver_height();
    if (panel_w <= 0 || panel_h <= 0) {
        return ESP_ERR_INVALID_STATE;
    }

    hub75_driver_fill_rgb(0, 0, (uint16_t)panel_w, (uint16_t)panel_h, 0, 0, 0);

    const int text_w = text_width_px(text);
    const int text_h = FONT_H * FONT_SCALE;
    int x = (panel_w - text_w) / 2;
    int y = (panel_h - text_h) / 2;

    for (const char *p = text; *p != '\0'; p++) {
        const uint8_t *glyph = glyph_for(*p);
        if (glyph != NULL) {
            draw_glyph(x, y, glyph, r, g, b);
        }
        x += (FONT_W + CHAR_SPACING) * FONT_SCALE;
    }

    ESP_LOGI(TAG, "draw centered \"%s\" on %dx%d", text, panel_w, panel_h);
    return ESP_OK;
}
