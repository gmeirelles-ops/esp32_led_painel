# Quickstart: Dashboard LVGL — Horário e Clima

**Feature**: `001-lvgl-clock-weather` | **Hardware**: Adafruit MatrixPortal S3 + HUB75 128×64

Validation guide for developers and hardware bring-up. Implementation details live in `tasks.md` after `/speckit-tasks`.

## Prerequisites

- ESP-IDF **v6.1** installed and exported (`idf.py --version`)
- Target `esp32s3` set
- MatrixPortal S3 + HUB75 panel connected (factory harness)
- Wi-Fi AP available for STA test
- Build directory outside repo (non-ASCII path):

  ```bash
  export IDF_BUILD_DIR=~/esp32_led_painel-build
  ```

## Related contracts

- Display: [contracts/hub75-display.md](./contracts/hub75-display.md)
- **Black panel debug**: [contracts/display-bringup-debug.md](./contracts/display-bringup-debug.md)
- NVS: [contracts/nvs-schema.md](./contracts/nvs-schema.md)
- Open-Meteo: [contracts/open-meteo-api.md](./contracts/open-meteo-api.md)
- Diagnostics: [contracts/diagnostics.md](./contracts/diagnostics.md)
- Data model: [data-model.md](./data-model.md)

## 1. Build (no flash)

From repository root:

```bash
idf.py -B ~/esp32_led_painel-build build
idf.py -B ~/esp32_led_painel-build size
```

**Expected**: Build succeeds with zero errors; size report shows sufficient free DRAM/heap headroom for LVGL task.

## 2. Configure Wi-Fi and city (lab)

Before first flash (user must authorize flash explicitly):

- Option A — menuconfig Kconfig defaults: `PAINEL_WIFI_SSID`, `PAINEL_WIFI_PASSWORD`, `PAINEL_DEFAULT_CITY`
- Option B — NVS partition write via documented helper (see tasks)

Example city for São Paulo region test: `São Paulo` with expected geocode lat/lon ≈ −23.55, −46.63.

## 3. Flash and monitor (requires explicit user authorization)

```bash
idf.py -B ~/esp32_led_painel-build flash monitor
```

**Do not run** without user approval per project policy.

## 4. Boot validation checklist

| Step | Action | Expected outcome | Spec ref |
|------|--------|------------------|----------|
| 4.1 | Power on with Wi-Fi configured | Splash `Iniciando...` then clock scene | US3 |
| 4.2 | Wait ≤ 2 min | Clock shows `HH:MM` matching local time ±1 min | SC-001, US1 |
| 4.3 | Wait ≤ 3 min | Weather scene shows temp + PT condition | SC-002, US2 |
| 4.4 | Observe 5 min | ≥ 4 alternations clock ↔ weather (~10 s each) | SC-006, US3 |
| 4.5 | Read panel at 1 m | Hour and temperature legible | SC-003 |

## 5. Connectivity failure scenarios

| Scenario | Setup | Expected UI | Expected log |
|----------|-------|---------------|--------------|
| Wrong Wi-Fi password | Invalid NVS pass | `Sem Wi-Fi` on both scenes | `PAINEL_DIAG_WIFI_AUTH_FAILED` |
| AP off | Valid cred, AP down | `Sem Wi-Fi` | reconnect backoff logs |
| SNTP blocked | Wi-Fi OK, NTP blocked | `Sem hora` on clock scene | `PAINEL_DIAG_SNTP_FAIL` |
| Empty city | Clear `city`, coords 0 | Weather: `Cidade?` | `PAINEL_DIAG_WEATHER_CONFIG_MISSING` |
| Offline after fetch | Wi-Fi off after success | Last weather + stale `*` | no crash 30 min | SC-004 |

## 6. Display hardware validation

If image corrupted or partial:

1. Confirm preset `adafruit-matrix-portal-s3` in logs
2. Try `shift_driver` alternate (document result in task notes) — **no GPIO changes**
3. If single 128×64 module fails but 64×32 tiles work, apply 2×2 layout from [research.md](./research.md) and re-test

## 6. Display hardware validation

If image corrupted or partial:

1. Confirm preset `adafruit-matrix-portal-s3` in logs
2. Try `shift_driver` alternate (document result in task notes) — **no GPIO changes**
3. If single 128×64 module fails but 64×32 tiles work, apply 2×2 layout from [research.md](./research.md) and re-test

## 9. Black panel — complete diagnostic run

**When**: Panel stays **fully black** but serial shows normal boot (Wi-Fi/clima OK).

**Contract**: [contracts/display-bringup-debug.md](./contracts/display-bringup-debug.md)

### Session A — No rebuild (5 min)

1. Flash existing firmware (user authorization required).
2. Open monitor; complete **Tier 0** checklist from contract § Tier 0.
3. Complete **Tier 1** electrical checks (power, cable, orientation).
4. Record results in the template at contract end.

**Stop if** Tier 0.1–0.3 missing → fix HUB75 init before any LVGL work.

### Session B — Bare-metal colors (requires bring-up firmware)

Prerequisites: `/speckit-implement` tasks for `CONFIG_PAINEL_DISPLAY_BRINGUP` merged.

```bash
idf.py -B ~/esp32_led_painel-build menuconfig
# Component config → Painel → Enable display bring-up sequence
idf.py -B ~/esp32_led_painel-build build
# flash + monitor (user authorization)
```

| Step | Watch panel | Pass |
|------|-------------|------|
| Boot | Full **red** ~3 s | Entire panel red |
| Next | Full **green** ~3 s | Entire panel green |
| Next | Full **blue** ~3 s | Entire panel blue |
| Next | Checkerboard | Grid visible |

- **All pass** → proceed Session C.
- **All fail** → Session D (matrix M1–M7), one variant per flash.

### Session C — LVGL smoke

Enable `CONFIG_PAINEL_DISPLAY_BRINGUP_LVGL`; rebuild, flash, monitor.

| Step | Expected |
|------|----------|
| White full screen | Panel predominantly white |
| Label `TEST` | Readable center text |
| Red 64×32 rect | Red block visible |

- **Pass** → Session E (normal app, bring-up Kconfig off).
- **Fail** → inspect `components/display_lvgl/display_lvgl.c` flush path.

### Session D — sdkconfig matrix (one change per flash)

Document each attempt. Start from **M0 baseline**, then:

| ID | Change | menuconfig path |
|----|--------|-----------------|
| M1 | Brightness 255 | HUB75 → Brightness |
| M2 | FM6126A shift | HUB75 → Shift driver |
| M4 | 1/8 32px scan wiring | HUB75 → Scan wiring |
| M5 | 64×32, 2×2 zigzag | HUB75 → Panel + Layout |

Re-run Session B after each change. **Do not** change GPIO pins.

### Session E — Normal dashboard

Disable all bring-up Kconfig options; full build; flash.

Run §4 boot checklist. Success = splash or clock/weather/error text — **never silent black** (SC-005).

### Quick reference — serial vs panel

| Serial | Panel | Next session |
|--------|-------|--------------|
| DMA started | Black | B or D |
| DMA started | Red in B | C |
| DMA + LVGL OK | Black in C | Fix flush |
| All OK | Black in E | App/scenes timing |

## 7. Unit tests (host / IDF)

```bash
idf.py -B ~/esp32_led_painel-build test
```

**Expected**: Open-Meteo JSON parse and WMO→PT mapping tests pass without hardware.

## 8. Success criteria mapping

| ID | Quickstart evidence |
|----|---------------------|
| SC-001 | Step 4.2 |
| SC-002 | Step 4.3 |
| SC-003 | Step 4.5 (subjective, record pass/fail) |
| SC-004 | Scenario offline 30 min |
| SC-005 | Section 5 all show on-screen error, not blank |
| SC-006 | Step 4.4 |

## Troubleshooting

| Symptom | Check |
|---------|-------|
| Blank panel | `PAINEL_DIAG_DISPLAY_INIT` in log; power/HUB75 cable; run **§9 Black panel diagnostic** |
| Panel black, serial OK | §9 Session A→E; likely scan or LVGL flush — see [display-bringup-debug.md](./contracts/display-bringup-debug.md) |
| LVGL crash | heap/PSRAM config; run `idf.py size` |
| Wrong time zone | NVS key `tz` in `painel_cfg` |
| Weather always `Cidade?` | NVS `city` or lat/lon; geocode logs |
| Build path errors | Use `-B ~/esp32_led_painel-build` |
