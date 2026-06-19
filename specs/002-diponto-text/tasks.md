# Tasks: DIPONTO Text Splash

## Phase 1: Reset

- [X] T001 Mark spec 001 superseded
- [X] T002 Create spec/plan 002-diponto-text
- [X] T003 Update constitution v2.0.0

## Phase 2: Minimal firmware

- [X] T004 Remove legacy components (app, connectivity, storage, protocol, display_lvgl)
- [X] T005 Simplify hub75_driver (FM6126A fixed, fill API)
- [X] T006 Implement diponto_font + centered DIPONTO in main/esp32_led_painel.c
- [X] T007 Update sdkconfig.defaults.esp32s3 (FM6126A, no bring-up/LVGL)
- [X] T008 Run idf.py build

## Phase 3: Hardware

- [X] T009 Flash and validate DIPONTO on panel — **PASS** (user confirmed 2026-06-19)
