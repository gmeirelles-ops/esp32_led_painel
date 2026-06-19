# Contract: HUB75 Display (MatrixPortal S3)

**Version**: 1.0 | **Feature**: `001-lvgl-clock-weather`

## Scope

Hardware display contract for Adafruit MatrixPortal S3 + factory HUB75 harness. **No custom GPIO map** in this repository.

## Board preset

| Property | Value |
|----------|-------|
| Preset ID | `adafruit-matrix-portal-s3` |
| Source | `esphome-libs/esp-hub75` → `Hub75BoardPreset::ADAFRUIT_MATRIXPORTAL_S3` (or equivalent API) |
| Target | ESP32-S3 GDMA |

## Panel topology

| Property | Value | Notes |
|----------|-------|-------|
| Logical resolution | 128 × 64 | LVGL canvas |
| Single panel config | `panel_width=128`, `panel_height=64` | Default for constitution "1× 128×64" |
| Alternate physical layout | 64×32 × 2×2 grid | Hardware validation only; same logical 128×64 |

If alternate layout required after hardware test, only these fields may change **without GPIO changes**:

- `panel_width`, `panel_height`
- `layout_rows`, `layout_cols`, `layout` (e.g. `TOP_LEFT_DOWN_ZIGZAG`)
- `shift_driver`, `scan_wiring`

## Driver settings (LVGL mode)

| Setting | Value |
|---------|-------|
| `double_buffer` | `false` |
| LVGL owns redraw | yes |
| `min_refresh_rate` | 60 Hz |
| Default brightness | 128 (0–255) |
| Pixel format | RGB565 |

## LVGL contract

| Setting | Value |
|---------|-------|
| `LV_COLOR_DEPTH` | 16 |
| Display size | 128 × 64 |
| LVGL heap | 48 KB (PSRAM preferred) |
| Primary font | Montserrat 24 |
| Secondary font | Montserrat 12 |
| Theme | Dark background, light text (high contrast) |

## Public driver API (component `hub75_driver`)

```c
esp_err_t hub75_driver_init(void);
esp_err_t hub75_driver_start(void);
uint16_t *hub75_driver_get_framebuffer(void);  // RGB565, 128*64
int hub75_driver_width(void);   // 128
int hub75_driver_height(void);  // 64
esp_err_t hub75_driver_set_brightness(uint8_t level);
```

## Failure behavior

- Init failure → `PAINEL_DIAG_DISPLAY_INIT`, log, `app_main` does not restart loop
- No fallback to custom pins
