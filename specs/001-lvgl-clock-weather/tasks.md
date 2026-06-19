---
description: "Task list for Dashboard LVGL — Horário e Clima"
---

# Tasks: Dashboard LVGL — Horário e Clima

**Input**: Design documents from `/specs/001-lvgl-clock-weather/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: Protocol unit tests included per plan.md (`test/protocol/`); not TDD-gated — implement after parsers exist (Polish phase).

**Organization**: Tasks grouped by user story (P1 horário → P2 clima → P3 alternância).

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: US1, US2, US3 — user story phases only
- Build command (all phases): `idf.py -B ~/esp32_led_painel-build build`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Component skeleton, dependencies, sdkconfig defaults

- [X] T001 Create ESP-IDF component directories per plan.md under `components/` (hub75_driver, display_lvgl, storage, connectivity, protocol, app)
- [X] T002 [P] Add `components/hub75_driver/CMakeLists.txt` and `components/hub75_driver/idf_component.yml` with `esphome-libs/esp-hub75` git dependency path `components/hub75`
- [X] T003 [P] Add `components/display_lvgl/CMakeLists.txt` and `components/display_lvgl/idf_component.yml` with `lvgl/lvgl` and `espressif/esp_lvgl_port` dependencies
- [X] T004 [P] Add stub `components/storage/CMakeLists.txt`, `components/connectivity/CMakeLists.txt`, `components/protocol/CMakeLists.txt`, `components/app/CMakeLists.txt` with `INCLUDE_DIRS` and empty/minimal sources
- [X] T005 [P] Update `sdkconfig.defaults.esp32s3` with SPIRAM, Wi-Fi STA, SNTP, HTTP client, and LVGL 16-bit color defaults per `specs/001-lvgl-clock-weather/research.md`
- [X] T006 Update `main/idf_component.yml` to depend on hub75_driver, display_lvgl, storage, connectivity, protocol, and app components
- [X] T007 Update `main/CMakeLists.txt` to register `esp32_led_painel.c` and declare component requirements

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Drivers, storage, connectivity, diagnostics — MUST complete before user story UI work

**⚠️ CRITICAL**: No user story work until this phase compiles

- [X] T008 Implement NVS key constants and schema v1 in `components/storage/painel_nvs_schema.h` mirroring `specs/001-lvgl-clock-weather/contracts/nvs-schema.md`
- [X] T009 Implement public storage API in `components/storage/include/painel_storage.h` (load/save config, wifi creds, weather cache)
- [X] T010 Implement `components/storage/painel_storage.c` with namespace `painel_cfg` and `wifi`, defaults (`America/Sao_Paulo`, dwell 10000 ms), and `PAINEL_DIAG_NVS_ERROR` handling
- [X] T011 [P] Implement `painel_diag_code_t` and log helpers in `components/app/include/painel_diag.h` per `specs/001-lvgl-clock-weather/contracts/diagnostics.md`
- [X] T012 Implement `components/hub75_driver/include/hub75_driver.h` and `components/hub75_driver/hub75_driver.c` using preset `adafruit-matrix-portal-s3`, panel 128×64, brightness 128, no custom GPIO
- [X] T013 Implement `components/display_lvgl/include/display_lvgl.h` and `components/display_lvgl/display_lvgl.c` with RGB565 flush to hub75 framebuffer, 48 KB LVGL heap (PSRAM), Montserrat 24/12 fonts, and `lvgl_task`
- [X] T014 Implement `components/connectivity/wifi_manager.c` and declarations in `components/connectivity/include/connectivity.h` (STA, NVS/Kconfig creds fallback, reconnect backoff, event queue to app)
- [X] T015 Implement `components/connectivity/sntp_service.c` in `components/connectivity/` (sync callback, TZ from NVS via `setenv`+`tzset`, `PAINEL_DIAG_SNTP_FAIL`)
- [X] T016 Implement `components/connectivity/http_client.c` in `components/connectivity/` (HTTPS GET wrapper, 10 s timeout, 4 KB buffer, retry backoff 5/15/45 s)
- [X] T017 [P] Implement `components/protocol/wmo_condition_pt.c` and `components/protocol/include/open_meteo.h` WMO→PT mapping (strings ≤12 chars)
- [X] T018 Implement minimal `components/app/include/dashboard_state.h`, `components/app/dashboard_state.c`, `components/app/include/app_controller.h`, and stub `components/app/app_controller.c` (init only, no scenes)
- [X] T019 Wire bootstrap sequence in `main/esp32_led_painel.c`: nvs_flash → storage → hub75 → display_lvgl → wifi → app_controller stub
- [X] T020 Add `main/Kconfig.projbuild` with `CONFIG_PAINEL_WIFI_SSID`, `CONFIG_PAINEL_WIFI_PASSWORD`, `CONFIG_PAINEL_DEFAULT_CITY` for lab provisioning
- [X] T021 Run `idf.py -B ~/esp32_led_painel-build build` and fix compile/link errors for foundational components

**Checkpoint**: Foundation builds — user story phases can begin

---

## Phase 3: User Story 1 — Ver horário correto no painel (Priority: P1) 🎯 MVP

**Goal**: Tela de horário legível com SNTP, fuso NVS, estados de erro `Sem hora` / `Sem Wi-Fi`

**Independent Test**: Wi-Fi + TZ configurados → hora local ±1 min em ≤2 min; sem Wi-Fi → mensagem de erro legível (spec US1)

### Implementation for User Story 1

- [X] T022 [P] [US1] Create `components/app/include/scene_clock.h` and `components/app/scene_clock.c` with LVGL labels for `HH:MM` (font 24) and optional `DD/MM` (font 12) per `specs/001-lvgl-clock-weather/plan.md` layout
- [X] T023 [US1] Implement clock sync state machine in `components/app/scene_clock.c` (`PENDING`/`SYNCED`/`ERROR`) consuming `sntp_service` status from `components/connectivity/sntp_service.c`
- [X] T024 [US1] Add 1 s `esp_timer` in `components/app/scene_clock.c` to refresh time labels from local TZ-aware `localtime_r`
- [X] T025 [US1] Implement clock error UI strings `Sem hora` and `Sem Wi-Fi` in `components/app/scene_clock.c` per `specs/001-lvgl-clock-weather/contracts/diagnostics.md`
- [X] T026 [US1] Start SNTP on `IP_EVENT_STA_GOT_IP` in `components/app/app_controller.c` and update clock scene on sync events
- [X] T027 [US1] Show clock scene as default active view from `components/app/app_controller.c` after display init (single-scene mode until US3)
- [X] T028 [US1] Add structured logs for SNTP/Wi-Fi failures with `painel_diag_code_t` in `components/connectivity/sntp_service.c` and `components/app/scene_clock.c`
- [X] T029 [US1] Run `idf.py -B ~/esp32_led_painel-build build` and verify US1 compiles; document hardware validation steps for spec US1 in task notes

**Checkpoint**: MVP clock screen — testable independently on hardware with Wi-Fi

---

## Phase 4: User Story 2 — Ver clima da cidade configurada (Priority: P2)

**Goal**: Tela de clima com Open-Meteo, cidade/coords NVS, cache offline, estados `Cidade?` / `Sem clima`

**Independent Test**: Cidade válida em NVS + Wi-Fi → temp + condição PT em ≤3 min; offline → último cache com indicador stale (spec US2)

### Implementation for User Story 2

- [X] T030 [P] [US2] Implement JSON geocode parser in `components/protocol/open_meteo_geocode.c` per `specs/001-lvgl-clock-weather/contracts/open-meteo-api.md`
- [X] T031 [P] [US2] Implement JSON forecast current-weather parser in `components/protocol/open_meteo_forecast.c` (`temperature_2m`, `weather_code`)
- [X] T032 [US2] Implement geocode-on-boot flow in `components/app/app_controller.c`: if `city` set and lat/lon zero, call geocode, persist coords via `components/storage/painel_storage.c`
- [X] T033 [US2] Implement weather fetch orchestration in `components/app/app_controller.c` using `components/connectivity/http_client.c` and protocol parsers; update `WeatherSnapshot` per `specs/001-lvgl-clock-weather/data-model.md`
- [X] T034 [US2] Implement weather cache read/write in `components/storage/painel_storage.c` (keys `wx_t_x10`, `wx_code`, `wx_ts`)
- [X] T035 [P] [US2] Create `components/app/include/scene_weather.h` and `components/app/scene_weather.c` with city (12 chars), temp `NN°C`, condition PT (12 chars) layout
- [X] T036 [US2] Implement stale cache indicator (`*` or `~`) and `source=CACHE` display logic in `components/app/scene_weather.c`
- [X] T037 [US2] Implement weather error states `Cidade?`, `Sem clima`, `Sem Wi-Fi` in `components/app/scene_weather.c`
- [X] T038 [US2] Add 30 min `esp_timer` periodic fetch in `components/app/app_controller.c` (`WEATHER_FETCH_INTERVAL_S=1800`)
- [X] T039 [US2] Load weather cache on boot in `components/app/app_controller.c` before first live fetch
- [X] T040 [US2] Run `idf.py -B ~/esp32_led_painel-build build` and verify US2 compiles

**Checkpoint**: Weather screen functional with live fetch and offline cache

---

## Phase 5: User Story 3 — Alternar entre horário e clima (Priority: P3)

**Goal**: Alternância automática 10 s, splash init, rotação mesmo com clima indisponível

**Independent Test**: 5 min observação → ≥4 ciclos clock↔weather; boot mostra `Iniciando...` (spec US3)

### Implementation for User Story 3

- [X] T041 [US3] Implement splash screen `Iniciando...` in `components/app/app_controller.c` or `components/display_lvgl/display_lvgl.c` until init complete
- [X] T042 [US3] Implement scene dwell timer (default 10 s, NVS `dwell_ms`) in `components/app/app_controller.c` toggling `CLOCK` ↔ `WEATHER` in `components/app/dashboard_state.c`
- [X] T043 [US3] Implement LVGL visibility switch between `components/app/scene_clock.c` and `components/app/scene_weather.c` containers in `components/app/app_controller.c`
- [X] T044 [US3] Ensure rotation continues when weather scene is in error state (show error screen, do not skip weather slot) in `components/app/app_controller.c`
- [X] T045 [US3] Dismiss splash and start rotation after first scene ready in `components/app/app_controller.c`
- [X] T046 [US3] Run `idf.py -B ~/esp32_led_painel-build build` and verify full dashboard flow compiles

**Checkpoint**: Full dashboard alternates clock and weather per spec SC-006

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Protocol tests, size gate, quickstart validation, hardware notes

- [X] T047 [P] Add Unity tests `test/protocol/test_wmo_pt.c` and `test/protocol/CMakeLists.txt` for WMO→PT mapping
- [X] T048 [P] Add Unity tests `test/protocol/test_open_meteo_parse.c` with JSON fixtures for geocode and forecast parsers
- [X] T049 Run `idf.py -B ~/esp32_led_painel-build build` and resolve any warnings relevant to new code
- [X] T050 Run `idf.py -B ~/esp32_led_painel-build size` and confirm heap/DRAM headroom ≥32 KB internal after init (log in task notes if below)
- [X] T051 Execute host/build tests via `idf.py -B ~/esp32_led_painel-build test` for protocol component if wired
- [X] T052 Walk through `specs/001-lvgl-clock-weather/quickstart.md` checklist; record hardware-only items pending user-authorized flash
- [X] T053 If 128×64 single-panel scan fails on hardware, document alternate 64×32 2×2 layout change in `components/hub75_driver/hub75_driver.c` only (no GPIO changes) per `specs/001-lvgl-clock-weather/research.md`

---

## Phase 7: Display Black Panel Bring-up (Priority: P0 debug)

**Goal**: Isolar painel preto com sequência HUB75 bare-metal + smoke LVGL opcional (plan Phase 2, [display-bringup-debug.md](./contracts/display-bringup-debug.md))

**Independent Test**: Flash com `CONFIG_PAINEL_DISPLAY_BRINGUP=y` → painel vermelho/verde/azul/checkerboard em ~12 s; serial `display_bringup: tier=2 step=N`

### Implementation for Display Bring-up

- [X] T054 Add `CONFIG_PAINEL_DISPLAY_BRINGUP`, `CONFIG_PAINEL_DISPLAY_BRINGUP_LVGL`, `CONFIG_PAINEL_DISPLAY_BRINGUP_HOLD_MS`, `CONFIG_PAINEL_DISPLAY_BRINGUP_HALT` in `main/Kconfig.projbuild`
- [X] T055 [P] Add `PAINEL_DIAG_DISPLAY_BRINGUP_FAIL` and `PAINEL_DIAG_DISPLAY_FLUSH_FAIL` in `components/storage/include/painel_diag.h` and `components/storage/painel_diag.c`
- [X] T056 Implement `hub75_driver_fill_rgb()` and `hub75_driver_run_bringup_sequence()` in `components/hub75_driver/include/hub75_driver.h` and `components/hub75_driver/hub75_driver.cpp`
- [X] T057 Implement `display_lvgl_run_smoke_test()` in `components/display_lvgl/include/display_lvgl.h` and `components/display_lvgl/display_lvgl.c`
- [X] T058 Wire conditional boot path in `main/esp32_led_painel.c` per `specs/001-lvgl-clock-weather/plan.md` Phase 2
- [X] T059 Enable lab defaults `CONFIG_PAINEL_DISPLAY_BRINGUP=y` and `CONFIG_PAINEL_DISPLAY_BRINGUP_HALT=y` in `sdkconfig.defaults.esp32s3`
- [X] T060 Run `idf.py -B ~/esp32_led_painel-build build` and `idf.py -B ~/esp32_led_painel-build size`
- [X] T061 Walk through `specs/001-lvgl-clock-weather/quickstart.md` §9 Session B; record hardware pass/fail in task notes

**Checkpoint**: Tier 2 color sequence runs on hardware; LVGL smoke available via menuconfig

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)** → **Foundational (Phase 2)** → **User Stories (Phases 3–5)** → **Polish (Phase 6)** → **Display bring-up (Phase 7)**
- **US2, US2, US3** require Phase 2 complete
- **US3** requires US1 + US2 scenes existing (integrates both; implement after US1/US2)

### User Story Dependencies

| Story | Depends on | Delivers independently |
|-------|------------|------------------------|
| **US1 (P1)** | Phase 2 | Clock only on panel — **MVP** |
| **US2 (P2)** | Phase 2; soft US1 (Wi-Fi shared) | Weather scene testable by forcing weather view before US3 |
| **US3 (P3)** | US1 + US2 scenes | Full alternation |

### Within Each User Story

- Storage/connectivity before scene UI
- Parsers before fetch orchestration (US2)
- Scenes before app_controller integration
- Build checkpoint at end of each story phase

### Parallel Opportunities

**Phase 1** (after T001): T002, T003, T004, T005 in parallel

**Phase 2** (after T008–T010): T011, T017 in parallel; T012 and T013 sequential (display depends on hub75)

**US1**: T022 parallel with nothing else in US1 until T023

**US2**: T030 and T031 in parallel; T035 parallel after T033 starts

**Polish**: T047 and T048 in parallel

---

## Parallel Example: User Story 2

```bash
# Parsers in parallel:
T030: components/protocol/open_meteo_geocode.c
T031: components/protocol/open_meteo_forecast.c

# After fetch orchestration (T033):
T035: components/app/scene_weather.c
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 + Phase 2
2. Complete Phase 3 (US1)
3. **STOP** — flash (with user authorization) and validate clock ±1 min per quickstart §4.2

### Incremental Delivery

1. Setup + Foundational → build green
2. US1 → clock MVP on hardware
3. US2 → weather + cache
4. US3 → alternation + splash
5. Polish → size, tests, quickstart

### Suggested MVP Scope

**Phases 1–3 (T001–T029)**: clock on 128×64 panel with SNTP — satisfies P1 and SC-001.

---

## Notes

- Never run `idf.py flash` or `erase_flash` without explicit user authorization (constitution V)
- Wi-Fi password and secrets must not appear in logs or committed configs
- GPIO pin map MUST NOT be added to repo — use hub75 preset only
- Mark tasks `[X]` in this file as each completes during `/speckit-implement`
