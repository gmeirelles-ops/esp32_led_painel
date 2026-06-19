# Tasks: Dashboard LVGL — Porto Alegre

**Feature**: `003-lvgl-poa-dashboard`

## Phase 1: Setup — [X]

- [X] T001 Constitution v2.1 amend
- [X] T002 Component skeleton + sdkconfig LVGL/Wi-Fi
- [X] T003 main Kconfig POA coords + Wi-Fi lab

## Phase 2: Foundational — [X]

- [X] T004 hub75_driver draw_pixels RGB565
- [X] T005 display_lvgl init + flush task
- [X] T006 storage NVS v2 (no city)
- [X] T007 connectivity Wi-Fi/SNTP/HTTP
- [X] T008 protocol Open-Meteo forecast + WMO PT

## Phase 3: US1 Clock — [X]

- [X] T009 scene_clock HH:MM + error states
- [X] T010 SNTP on GOT_IP

## Phase 4: US2 Weather POA — [X]

- [X] T011 scene_weather Porto Alegre + cache offline
- [X] T012 30 min periodic fetch

## Phase 5: US3 Rotation — [X]

- [X] T013 splash + 10 s dwell timer

## Phase 6: Validation — [X]

- [X] T014 `idf.py -B ~/esp32_led_painel-build build` PASS

**Hardware validation**: pending user-authorized flash on MatrixPortal S3.
