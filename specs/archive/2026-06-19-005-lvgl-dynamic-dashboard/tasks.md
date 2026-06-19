# Tasks: Dashboard LVGL Dinâmico

**Feature**: `005-lvgl-dynamic-dashboard` | **Plan**: [plan.md](./plan.md)

## Phase 1 — Setup

- [X] T001 Enable `CONFIG_LV_FONT_MONTSERRAT_48` in `sdkconfig.defaults.esp32s3`

## Phase 2 — Clock fix (P1)

- [X] T002 `connectivity_bootstrap()` in `app_controller.c` — handle Wi-Fi already up + SNTP synced
- [X] T003 SNTP sync log local time in `wifi_manager.c`
- [X] T004 `scene_clock.c` — pending state "Sincronizando…", validate tm_year, never empty fields

## Phase 3 — Responsive layout (P1)

- [X] T005 Add `scene_layout.c` / `include/scene_layout.h` with compact/large profiles
- [X] T006 Update `scene_clock.c` to use `scene_layout_get()`
- [X] T007 Update `scene_weather.c` to use `scene_layout_get()` + stale opacity
- [X] T008 Register `scene_layout.c` in `components/app/CMakeLists.txt`

## Phase 4 — Transitions (P2)

- [X] T009 `apply_scene_animated()` fade 250+250 ms in `app_controller.c`
- [X] T010 Transition logs per `contracts/scene-transitions.md`

## Phase 5 — Validation

- [X] T011 `idf.py -B ~/esp32_led_painel-build build` PASS
