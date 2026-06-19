#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SCENE_LAYOUT_COMPACT = 0,
    SCENE_LAYOUT_LARGE,
} scene_layout_tier_t;

typedef struct {
    scene_layout_tier_t tier;
    lv_coord_t hor;
    lv_coord_t ver;
    lv_coord_t clock_hero_h;
    lv_coord_t weather_band_w;
    const lv_font_t *font_hero;
    const lv_font_t *font_body;
    const lv_font_t *font_caption;
    lv_color_t color_bg;
    lv_color_t color_panel;
    lv_color_t color_clock_hero;
    lv_color_t color_weather_band;
    lv_color_t color_on_hero;
    lv_color_t color_text;
    lv_color_t color_muted;
} scene_layout_profile_t;

scene_layout_profile_t scene_layout_get(void);
void scene_layout_init_root(lv_obj_t *root, const scene_layout_profile_t *layout);
lv_obj_t *scene_layout_panel(lv_obj_t *root, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                               lv_color_t bg);

#ifdef __cplusplus
}
#endif
