# Contract: UI Layout

**Version**: 1.0 | **Feature**: `005-lvgl-dynamic-dashboard`

## Breakpoints

| Profile | Canvas | Primary font | Secondary font |
|---------|--------|--------------|----------------|
| compact | 128×64 | Montserrat 24 | Montserrat 12 |
| large | 256×128 | Montserrat 48 | Montserrat 24 |

Breakpoint: `display_lvgl_hor_res() > 128` → large.

## Visual rules

- Background: black (`#000000`)
- Text: white (`#FFFFFF`)
- Stale weather: 70% opacity on temperature label
- Error text: Montserrat secondary, centered
- Root object: full canvas size, no border, pad from profile

## API (internal)

```c
typedef enum { SCENE_LAYOUT_COMPACT, SCENE_LAYOUT_LARGE } scene_layout_tier_t;

typedef struct {
    scene_layout_tier_t tier;
    const lv_font_t *font_primary;
    const lv_font_t *font_secondary;
    int16_t pad_outer;
    int16_t time_y_offset;
    int16_t date_y_offset;
} scene_layout_profile_t;

scene_layout_profile_t scene_layout_get(void);
```

## Constitution amendment

Milestone 1 UI contract extended: large profile adds Montserrat 48 for 256×128 builds; compact unchanged.
