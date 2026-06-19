<!--
Sync Impact Report
- Version change: 2.0.0 → 2.1.0
- Modified principles:
  - I. Hardware Contracts → FM6126A/128×64/10 MHz preserved; LVGL dashboard reactivated
  - III. Field Diagnosability → Wi-Fi/SNTP/HTTP/display/LVGL logs required for milestone 1
  - IV. Clean Architecture → components: hub75_driver, display_lvgl, storage, connectivity, protocol, app
  - V. Validation → dashboard clock+weather POA hardware criterion added
- Added sections: Milestone 1 (LVGL POA dashboard), Wi-Fi/SNTP/HTTP contracts active
- Removed from deferred: LVGL dashboard, Wi-Fi, SNTP, HTTP, weather cache NVS
- Templates: plan-template.md ✅ no change; spec-template.md ✅; tasks-template.md ✅
- Feature 003-lvgl-poa-dashboard active; 002-diponto-text hardware baseline preserved
-->

# esp32_led_painel Constitution

## Core Principles

### I. Hardware Contracts Are Stable

GPIO pinout, electrical function, bootstrapping constraints, bus frequency, partition layout, NVS schema, OTA strategy, TLS configuration, MQTT topics, payload schemas, QoS, retain, LWT and keepalive, codec selection, I2S pinout, board config and audio pipeline topology (when applicable) are compatibility contracts. They MUST NOT change without explicit requirement in `spec.md`, `plan.md` and `tasks.md`, documented rationale, migration impact and validation plan.

HUB75 display contracts (validated in **Milestone 0**, preserved in **Milestone 1**):

- **Board**: Adafruit MatrixPortal S3 (`esp32s3`) with factory HUB75 harness — no custom GPIO pin map in this repository; pin assignment is the board preset (`esphome/esp-hub75` MatrixPortal profile)
- **Panel topology**: 1× HUB75 module, **128×64** pixels, scan **1/32**, interface **HUB75E** (pin E via preset; e.g. SXHL-P2.5-1735 class panels)
- **Display driver**: `esphome/esp-hub75` with MatrixPortal preset, DMA GDMA on ESP32-S3
- **Shift driver**: **`Hub75ShiftDriver::FM6126A`** — fixed; changes require spec/plan amendment
- **Output clock**: **10 MHz** default (`sdkconfig.defaults.esp32s3`); changes require spec/plan amendment
- **PSRAM**: quad mode (`CONFIG_SPIRAM_MODE_QUAD`); octal MUST NOT be enabled (conflicts with HUB75 GPIO on MatrixPortal)

**Milestone 1 UI contract**:

- LVGL canvas **128×64**, RGB565, flush via `hub75_driver` `draw_pixels(RGB565)`
- Fonts: Montserrat 24 (hora/temp), 12 (rótulos/erro)
- Brightness default: 255 (validated on hardware); changes require plan amendment

MQTT, OTA, BLE/Blufi remain **out of scope** until a future approved feature.

### II. Spec First, Code Second

Feature work follows the Spec Kit flow: constitution, specify, clarify when useful, checklist, plan, tasks and implement. Specification, planning and task-generation phases MUST NOT implement code. Implementation MUST follow approved `spec.md`, `plan.md` and `tasks.md`; if implementation reveals a requirement, architecture or hardware-contract change, update the Spec Kit artifacts before continuing.

### III. Field Diagnosability Is Required

Firmware MUST produce actionable diagnostics for real hardware and field support.

**Minimum logs** (tags `main`, `hub75_drv`, `display_lvgl`, `wifi_mgr`, `sntp_svc`, `app_ctrl`):

- HUB75 init dimensions, FM6126A success/failure
- LVGL init success/failure
- Wi-Fi connect/disconnect reason, masked SSID
- SNTP sync success/failure
- HTTP weather fetch success/failure (no credentials)
- Scene transitions and error states

Structured diagnostic enum `painel_diag_code_t` per feature contracts. MQTT/app payloads deferred.

### IV. Embedded Clean Architecture, Pragmatically Applied

**Milestone 1** component boundaries:

- `components/hub75_driver/` — HUB75 hardware (preset pins, FM6126A, draw_pixels)
- `components/display_lvgl/` — LVGL port, flush task
- `components/storage/` — NVS (wifi creds, weather cache, tz, dwell)
- `components/connectivity/` — Wi-Fi STA, SNTP, HTTPS GET
- `components/protocol/` — Open-Meteo forecast parse, WMO→PT
- `components/app/` — scenes (clock, weather), dashboard rotation, orchestration
- `main/` — bootstrap only

Domain logic MUST NOT live in Wi-Fi callbacks or HTTP handlers; events signal `app_controller`.

### V. Validation Before Done

Every code change MUST be validated with the strongest practical check available.

- `idf.py -B ~/esp32_led_painel-build build` is **mandatory** for all firmware changes
- `idf.py size` recommended when LVGL/connectivity changes heap/partition usage
- `idf.py flash`, `idf.py erase_flash`, destructive NVS operations MUST NOT run without explicit user authorization
- **Milestone 1 hardware done criterion**: clock and Porto Alegre weather legible at ~1 m with Wi-Fi stable; auto-rotation clock↔weather every 10 s

## Project Constraints

| Constraint | Value |
|------------|-------|
| Framework | ESP-IDF v6.1, target **esp32s3** |
| Board | Adafruit MatrixPortal S3 + factory HUB75 harness |
| Display | 1× HUB75 **128×64**, FM6126A, 10 MHz clock |
| UI | LVGL dashboard: horário + clima **Porto Alegre** (fixo) |
| Location | Porto Alegre, `America/Sao_Paulo` (fixo) |
| Active milestone | `003-lvgl-poa-dashboard` |
| Connectivity | Wi-Fi STA, SNTP, HTTPS Open-Meteo (no MQTT/OTA) |
| Build gate | `idf.py -B ~/esp32_led_painel-build build` |
| Flash policy | No flash/erase without explicit user authorization |

Secrets MUST NEVER be committed to the repository or feature specs.

### Milestone 1 — LVGL dashboard Porto Alegre

On boot with Wi-Fi available:

1. Initialize HUB75 (FM6126A) + LVGL 128×64
2. Sync time via SNTP (`America/Sao_Paulo`)
3. Fetch weather for fixed Porto Alegre coordinates
4. Alternate **clock** and **weather** screens every **10 s**
5. Show legible error states when offline or sync fails

### Deferred (explicitly out of scope)

- MQTT, OTA, BLE/Blufi provisioning
- Configurable city / geocoding NVS
- Custom partition table changes beyond `SINGLE_APP_LARGE`
- Field diagnostics MQTT contract and app Flutter integration
- Multi-profile display bring-up debug firmware (unless regression)

## Cross-Platform Development

Spec Kit scripts in `.specify/scripts/bash/` and `.specify/scripts/powershell/`. Keep specs portable; avoid OS-specific paths unless required.

## Governance

Amendments REQUIRE documented reason, compatibility impact and validation plan. `AGENTS.md` MUST NOT weaken these principles.

Feature `002-diponto-text` hardware validation remains authoritative for HUB75 contracts. Feature `003-lvgl-poa-dashboard` is the active product milestone.

**Version**: 2.1.0 | **Ratified**: 2026-06-14 | **Last Amended**: 2026-06-18
