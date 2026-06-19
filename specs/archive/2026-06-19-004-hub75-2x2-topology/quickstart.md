# Quickstart: HUB75 Display Topology

## Prerequisites

- ESP-IDF v6.1, target `esp32s3`
- Adafruit MatrixPortal S3
- Build dir: `~/esp32_led_painel-build`

## Profile Single (default)

```bash
idf.py -B ~/esp32_led_painel-build build
```

Expected: virtual 128×64, dashboard unchanged from milestone 003.

## Profile 2×2

1. `idf.py menuconfig` → **Painel LED — configuração** → **Display topology** → **Four panels 2×2 (256×128)**
2. Save and build:

```bash
idf.py -B ~/esp32_led_painel-build build
idf.py -B ~/esp32_led_painel-build size
```

3. Flash only with explicit authorization.

## Serial verification

Look for:

```
I (…) hub75_board: topology 2x2 panel 128x64 layout 2x2 virtual 256x128
I (…) hub75_drv: display 256x128
I (…) display_lvgl: LVGL 256x128 ready (full frame, core 1, fb 65536 bytes, …)
```

## 2×2 wiring reference

Four 128×64 modules in 2×2 grid, HUB75 chain, zigzag layout (all panels upright):

```text
+-------+-------+
| P0    | P1    |  top row: data flows P0 → P1
+-------+-------+
| P2    | P3    |  bottom row: P1 → P2 → P3 (zigzag)
+-------+-------+
```

If quadrants appear swapped, try serpentine layout (future Kconfig) or verify physical chain order.

## Tuning

If flicker on 2×2: `menuconfig` → HUB75 → Minimum Refresh Rate → 30 Hz.

## Related

- [hub75-display-topology.md](./contracts/hub75-display-topology.md)
- [data-model.md](./data-model.md)
