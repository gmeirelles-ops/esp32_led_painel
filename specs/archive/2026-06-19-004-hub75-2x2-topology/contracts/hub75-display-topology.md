# Contract: HUB75 Display Topology

**Version**: 1.0 | **Feature**: `004-hub75-2x2-topology`

## Scope

Build-time display topology for MatrixPortal S3 + factory HUB75 harness. GPIO preset unchanged.

## Profiles

| Profile | Kconfig | panel | layout | layout type | virtual |
|---------|---------|-------|--------|-------------|---------|
| Single (default) | `CONFIG_PAINEL_TOPOLOGY_SINGLE` | 128×64 | 1×1 | HORIZONTAL | 128×64 |
| 2×2 | `CONFIG_PAINEL_TOPOLOGY_2X2` | 128×64 | 2×2 | TOP_LEFT_DOWN_ZIGZAG | 256×128 |

## Fixed across profiles

| Setting | Value |
|---------|-------|
| Board preset | Adafruit MatrixPortal S3 |
| Shift driver | FM6126A |
| Output clock | 10 MHz |
| Scan wiring | STANDARD_TWO_SCAN |
| GPIO | MatrixPortal preset (no repo overrides) |

## Kconfig location

`menuconfig` → **Painel LED — configuração** → **Display topology**

Project Kconfig is the source of truth for `layout_rows`, `layout_cols`, and `layout` when `PAINEL_TOPOLOGY_*` is set. Component-level `HUB75_LAYOUT_ROWS/COLS` are overridden by `hub75_board_config.h`.

## LVGL contract

| Setting | Single | 2×2 |
|---------|--------|-----|
| Canvas size | 128×64 | 256×128 |
| Color depth | RGB565 | RGB565 |
| Framebuffer | PSRAM, dynamic | PSRAM, dynamic |
| Fonts | Montserrat 24/12 | Montserrat 24/12 |

## Public API (unchanged signatures)

```c
int hub75_driver_width(void);   // virtual width
int hub75_driver_height(void);  // virtual height
int display_lvgl_hor_res(void);
int display_lvgl_ver_res(void);
```
