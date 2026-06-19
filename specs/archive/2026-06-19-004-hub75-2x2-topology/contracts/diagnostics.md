# Contract: Diagnostics — Display Topology

**Version**: 1.0 | **Feature**: `004-hub75-2x2-topology`

## Boot logs (required)

### hub75_board

```
topology single panel 128x64 layout 1x1 virtual 128x64
```

or

```
topology 2x2 panel 128x64 layout 2x2 virtual 256x128
```

### hub75_drv

```
display 128x64    # single
display 256x128   # 2x2
```

### display_lvgl

```
LVGL 128x64 ready (full frame, core 1, fb N bytes, free heap X)
```

Framebuffer size `N` = `width × height × 2`.

## Failure behavior

| Condition | Log | Result |
|-----------|-----|--------|
| HUB75 init fail | `hub75 begin failed` | `ESP_FAIL` from `hub75_driver_init` |
| LVGL PSRAM alloc fail | `LVGL framebuffer alloc failed` | `ESP_ERR_NO_MEM` |
| Invalid driver dimensions | `invalid display size` | `ESP_ERR_INVALID_STATE` |

## Hardware validation checklist (2×2)

- [ ] Four quadrants show correct content (no swapped panels)
- [ ] Full-frame fill colors match expected zigzag mapping
- [ ] Dashboard clock/weather legible at ~1 m
- [ ] No abnormal flicker at 60 Hz min refresh (tune if needed)
