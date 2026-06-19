#include "scene_layout.h"
#include "display_lvgl.h"
#include "sdkconfig.h"

static void fill_profile(scene_layout_profile_t *p, bool large)
{
    p->hor = display_lvgl_hor_res();
    p->ver = display_lvgl_ver_res();
    p->color_bg = lv_color_black();
    p->color_panel = lv_color_hex(0x0C1420);
    p->color_clock_hero = lv_color_hex(0x0088CC);
    p->color_weather_band = lv_color_hex(0xE07000);
    p->color_on_hero = lv_color_white();
    p->color_text = lv_color_white();
    p->color_muted = lv_color_hex(0x8CA0B8);

    if (large) {
        p->tier = SCENE_LAYOUT_LARGE;
#if CONFIG_LV_FONT_MONTSERRAT_48
        p->font_hero = &lv_font_montserrat_48;
#else
        p->font_hero = &lv_font_montserrat_24;
#endif
        p->font_body = &lv_font_montserrat_24;
        p->font_caption = &lv_font_montserrat_14;
        p->clock_hero_h = (lv_coord_t)(p->ver * 55 / 100);
        p->weather_band_w = 18;
    } else {
        p->tier = SCENE_LAYOUT_COMPACT;
        p->font_hero = &lv_font_montserrat_24;
        p->font_body = &lv_font_montserrat_14;
        p->font_caption = &lv_font_montserrat_12;
        p->clock_hero_h = 38;
        p->weather_band_w = 10;
    }
}

scene_layout_profile_t scene_layout_get(void)
{
    scene_layout_profile_t profile;
    fill_profile(&profile, display_lvgl_hor_res() > 128);
    return profile;
}

void scene_layout_init_root(lv_obj_t *root, const scene_layout_profile_t *layout)
{
    lv_obj_set_style_bg_color(root, layout->color_bg, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *scene_layout_panel(lv_obj_t *root, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                               lv_color_t bg)
{
    lv_obj_t *panel = lv_obj_create(root);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_bg_color(panel, bg, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}
