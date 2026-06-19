# Implementation Plan: DIPONTO Text Splash

**Branch**: `002-diponto-text` | **Date**: 2026-06-19 | **Spec**: [spec.md](./spec.md)

## Summary

Minimal ESP-IDF firmware: `hub75_driver` (MatrixPortal preset, FM6126A, 128×64) + bitmap font in `main/` to render **DIPONTO** centered. No LVGL.

## Technical Context

- ESP-IDF v6.1, esp32s3, MatrixPortal S3
- `esphome/esp-hub75`, clock 10 MHz, FM6126A
- PSRAM quad mode in `sdkconfig.defaults.esp32s3`

## Structure

```text
main/
  esp32_led_painel.c
  diponto_font.c / diponto_font.h
components/
  hub75_driver/
```

## Constitution Check

| Principle | Status |
|-----------|--------|
| I Hardware | PASS — FM6126A, 128×64, preset pins |
| II Spec first | PASS |
| III Diagnostics | PASS — hub75/FM6126A logs |
| IV YAGNI | PASS — two locations only |
| V Validation | PASS — idf.py build + hardware DIPONTO |
