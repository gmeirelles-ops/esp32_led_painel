# Contract: Display Bring-up Debug (Black Panel)

**Version**: 1.0 | **Feature**: `001-lvgl-clock-weather` | **Date**: 2026-06-18

## Problem statement

Firmware boots cleanly on serial (Wi-Fi, SNTP, Open-Meteo OK) but the HUB75 panel remains **fully black** — no splash, clock, weather or error text visible.

This contract defines a **layered isolation program** to locate the failure between electrical bring-up, HUB75 DMA/scan, LVGL flush, and application UI — without changing GPIO pin assignments.

## Scope

| In scope | Out of scope |
|----------|--------------|
| Serial log audit | Custom GPIO map |
| Kconfig-only HUB75 tuning (shift driver, wiring, layout, brightness) | `erase_flash` / destructive NVS |
| Optional firmware **display bring-up mode** (solid fills, patterns, LVGL smoke test) | MQTT / OTA / BLE |
| Pass/fail recording per tier | Permanent product UI changes |

## Isolation model

```text
[T0 Serial] → [T1 Electrical] → [T2 HUB75 bare-metal] → [T3 LVGL smoke] → [T4 App UI] → [T5 Scan matrix]
     │              │                    │                    │               │
  stack alive?   power/cable?      DMA writes pixels?    flush path OK?   wrong topology?
```

**Decision rule**: Stop at the **first tier that fails** and fix before advancing. If tier passes but panel still black, the fault is **downstream** (e.g. T2 pass + T3 fail → LVGL/flush).

## Tier 0 — Serial boot audit (no flash change)

Run monitor after authorized flash. Record pass/fail for each marker:

| # | Log marker (tag) | Meaning | Pass if |
|---|------------------|---------|---------|
| 0.1 | `hub75_drv: display 128x64` | Driver dimensions | Present |
| 0.2 | `Hub75Driver: Driver started successfully` | Shift init + DMA start | Present |
| 0.3 | `GdmaDma: Descriptor-chain DMA transfer started` | Continuous refresh | Present |
| 0.4 | `display_lvgl: lvgl 128x64 buf=4096 bytes` | LVGL init | Present |
| 0.5 | `main: running` | App reached steady state | Present |
| 0.6 | No `PAINEL_DIAG_DISPLAY_INIT` / Guru Meditation | No display crash | Absent |

**Interpretation**:

- **0.1–0.3 fail** → HUB75 init/DMA; fix before LVGL.
- **0.1–0.3 pass, panel black** → DMA runs but framebuffer likely zero or scan wrong → **T2 mandatory**.
- **0.4 fail** → LVGL/memory; panel may stay black even if HUB75 OK.

## Tier 1 — Electrical and mechanical (hardware only)

| # | Check | Expected |
|---|-------|----------|
| 1.1 | Panel 5 V supply (Adafruit spec) | Stable under load |
| 1.2 | HUB75 harness fully seated (factory MatrixPortal cable) | No loose IDC |
| 1.3 | Panel power LED (if present) | On |
| 1.4 | No swapped INPUT/OUTPUT between boards | Correct orientation |

If 1.1–1.4 suspect → resolve before software tiers. A fully dead panel (no flicker ever) often implicates **1.x** or **T5 shift driver**.

## Tier 2 — HUB75 bare-metal (firmware bring-up mode)

**Requires**: `CONFIG_PAINEL_DISPLAY_BRINGUP=y` (see [plan.md](../plan.md) Phase 2).

After `hub75_driver_init()` / `begin()`, **before** `display_lvgl_init()`:

| Step | Action | Visual expected | Isolates |
|------|--------|-----------------|----------|
| 2.1 | `fill(0,0,128,64,255,0,0)` full red, 3 s hold | Entire panel red | DMA + scan + power |
| 2.2 | Full green 3 s | Entire panel green | RGB channel wiring |
| 2.3 | Full blue 3 s | Entire panel blue | RGB channel wiring |
| 2.4 | Checkerboard 8×8 (white/black) | Visible grid | Addressing / partial scan |
| 2.5 | `set_brightness(255)` repeat 2.1 | Brighter red | OE / brightness |

Log each step: `display_bringup: step=N result=PASS|FAIL`.

**Outcomes**:

| T2 result | Next action |
|-----------|-------------|
| **All colors visible** | Fault is **LVGL or app** → T3 |
| **Black always** | T5 scan matrix + re-check T1 |
| **Wrong colors / stripes / partial** | T5 layout/wiring/shift driver |
| **Dim but correct hue** | Raise brightness; check 5 V |

### Public API additions (component `hub75_driver`)

```c
esp_err_t hub75_driver_fill_rgb(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                uint8_t r, uint8_t g, uint8_t b);
esp_err_t hub75_driver_run_bringup_sequence(void);  /* steps 2.1–2.4 when Kconfig enabled */
```

Wrappers around upstream `Hub75Driver::fill()` — no GPIO changes.

## Tier 3 — LVGL smoke (minimal UI)

With bring-up mode **stage LVGL only** (`CONFIG_PAINEL_DISPLAY_BRINGUP_LVGL=y`):

| Step | LVGL content | Visual expected |
|------|--------------|-----------------|
| 3.1 | Full-screen `lv_obj` style bg **white** (`0xFFFF`) | Panel mostly white |
| 3.2 | Center label `"TEST"` font 24, **black** text | Readable "TEST" |
| 3.3 | Red rectangle 64×32 centered | Red block on white |

**Outcomes**:

| T2 | T3 | Diagnosis |
|----|-----|-----------|
| PASS | FAIL | Flush callback, stride, mutex, or LVGL buffer — inspect `display_lvgl.c` |
| PASS | PASS | HUB75 + LVGL OK — fault in **app scenes** → T4 |

## Tier 4 — Application UI

Restore normal firmware (bring-up Kconfig off). Verify:

| Step | Check | Expected on panel |
|------|-------|-------------------|
| 4.1 | Boot splash | `"Iniciando..."` visible ≤ 5 s |
| 4.2 | Clock scene (Wi-Fi + SNTP OK) | `HH:MM` white on dark |
| 4.3 | Weather scene | Temp + condition |
| 4.4 | Wi-Fi off | `"Sem Wi-Fi"` — **must not stay blank** (SC-005) |

If T3 pass but T4.1 fail: scene creation order, `display_lvgl_start()` timing, or objects hidden (`LV_OBJ_FLAG_HIDDEN`).

## Tier 5 — HUB75 config matrix (sdkconfig only)

Apply **one change at a time**; rebuild + flash; record result. **Never change GPIO pins.**

| Variant | Kconfig changes | When to try |
|---------|-----------------|-------------|
| M0 baseline | Current: 128×64, GENERIC, STANDARD wiring, layout 1×1 HORIZONTAL | First T2 fail |
| M1 brightness | `CONFIG_HUB75_BRIGHTNESS=255` | Dim or black with faint flicker |
| M2 shift FM6126A | `CONFIG_HUB75_DRIVER_FM6126A=y` | Common on 64×32 / some 128×64 modules |
| M3 shift FM6124 | `CONFIG_HUB75_DRIVER_FM6124=y` | M2 no improvement |
| M4 wiring 1/8 32px | `CONFIG_HUB75_WIRING_SCAN_1_8_32PX=y` | Horizontal banding |
| M5 layout 2×2 | `PANEL 64×32`, `LAYOUT_ROWS=2`, `LAYOUT_COLS=2`, `TOP_LEFT_DOWN_ZIGZAG` | Physically four 64×32 tiles |
| M6 clock 16 MHz | `CONFIG_HUB75_CLK_16MHZ=y` | Glitchy/unstable image |
| M7 bit depth 7 | `CONFIG_HUB75_BIT_DEPTH_7=y` | lsbMsb warning / color collapse |

Canvas LVGL stays **128×64** for all variants (logical resolution unchanged per [hub75-display.md](./hub75-display.md)).

## Recording template

Copy into task notes or lab log:

```text
Date:
Firmware git:
Tier 0: [PASS/FAIL] notes:
Tier 1: [PASS/FAIL] notes:
Tier 2 step 2.1 red: [PASS/FAIL]
Tier 3 white fill: [PASS/FAIL]
Tier 4 splash: [PASS/FAIL]
Best matrix variant: M?
Serial excerpt:
```

## Success criteria (debug program complete)

Debug program is **complete** when:

1. Tier 2.1 (full red) **PASS** on hardware, **or**
2. Root cause documented with evidence (e.g. "M2 FM6126A fixes scan; clock scene visible"), **and**
3. Normal app (T4) shows clock or explicit on-screen error — never silent black.

## Related artifacts

- [hub75-display.md](./hub75-display.md) — stable display contract
- [diagnostics.md](./diagnostics.md) — `painel_diag_code_t` extensions for bring-up
- [quickstart.md](../quickstart.md) §9 — runnable procedure
- [research.md](../research.md) §11 — root-cause decisions
