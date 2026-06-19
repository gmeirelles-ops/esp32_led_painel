# Data Model: Display Topology

## display_topology_profile (build-time)

Compile-time profile selected via Kconfig; **not** stored in NVS.

| Field | Type | Source | Notes |
|-------|------|--------|-------|
| profile | enum | `CONFIG_PAINEL_TOPOLOGY_*` | `SINGLE` or `2X2` |
| panel_width | uint16 | derived | Always 128 |
| panel_height | uint16 | derived | Always 64 |
| layout_rows | uint16 | derived | 1 (single) or 2 (2×2) |
| layout_cols | uint16 | derived | 1 (single) or 2 (2×2) |
| layout_type | enum | derived | HORIZONTAL or TOP_LEFT_DOWN_ZIGZAG |
| virtual_width | uint16 | computed | `panel_width × layout_cols` |
| virtual_height | uint16 | computed | `panel_height × layout_rows` |

## Validation rules

- `layout_rows` and `layout_cols` MUST be ≥ 1
- For `SINGLE`: `layout_rows=1`, `layout_cols=1`, virtual 128×64
- For `2X2`: `layout_rows=2`, `layout_cols=2`, virtual 256×128
- Profile MUST NOT change at runtime

## State transitions

N/A — immutable after firmware build.

## Relationships

- `hub75_driver` reads profile via `hub75_get_matrixportal_config()`
- `display_lvgl` queries virtual dimensions via `hub75_driver_width/height()`
- LVGL scenes query `display_lvgl_hor_res/ver_res()`
