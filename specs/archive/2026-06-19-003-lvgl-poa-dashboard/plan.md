# Implementation Plan: Dashboard LVGL — Porto Alegre

**Branch**: `003-lvgl-poa-dashboard` | **Date**: 2026-06-18 | **Spec**: [spec.md](./spec.md)

## Summary

Firmware ESP-IDF v6.1 para MatrixPortal S3: dashboard LVGL **128×64** com tela de **horário** (SNTP, `America/Sao_Paulo`) e **clima fixo de Porto Alegre** (Open-Meteo), alternância **10 s**. Preserva contratos HUB75 validados na 002: **FM6126A**, **10 MHz**, preset MatrixPortal.

## Technical Context

- **Target**: esp32s3, ESP-IDF 6.1, quad PSRAM
- **UI**: LVGL 9.x, RGB565 flush via `hub75_driver_draw_pixels_rgb565`
- **Location**: lat `-30.0346`, lon `-51.2177` (Kconfig `CONFIG_PAINEL_POA_*`)
- **Wi-Fi**: STA, creds NVS `wifi` ou Kconfig lab fallback
- **Storage**: NVS `painel_cfg` (tz, dwell, weather cache v2) — sem cidade/geocode
- **Build**: `idf.py -B ~/esp32_led_painel-build build`

## Constitution Check

| Princípio | Status |
|-----------|--------|
| I. Hardware contracts | PASS — FM6126A/128×64/10 MHz unchanged |
| II. Spec first | PASS |
| III. Diagnosability | PASS — logs wifi/sntp/http/display |
| IV. Clean architecture | PASS — 6 components |
| V. Validation | PASS — build green |

## Project Structure

```text
components/hub75_driver/   # FM6126A, draw_pixels RGB565
components/display_lvgl/   # LVGL task + flush
components/storage/        # NVS wifi + cache
components/connectivity/   # Wi-Fi, SNTP, HTTP
components/protocol/       # Open-Meteo parse, WMO→PT
components/app/            # scenes + app_controller
main/                      # bootstrap
```

## Initialization

```text
nvs → hub75 → display_lvgl → wifi → app_controller
  → [GOT_IP] sntp → weather fetch POA → scene rotation
```
