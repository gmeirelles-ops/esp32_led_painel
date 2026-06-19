<!--
Sync Impact Report
- Version change: 1.1.0 → 2.0.0
- Modified principles:
  - I. Hardware Contracts → FM6126A fixed; 128×64 HUB75E; no LVGL contract
  - III. Field Diagnosability → hub75/FM6126A logs only for milestone 0
  - IV. Clean Architecture → YAGNI: main + hub75_driver only
  - V. Validation → DIPONTO visible on hardware is done criterion
- Added sections: Milestone 0 (DIPONTO text), explicit deferred list
- Removed sections: LVGL/UI dashboard as active milestone; Wi-Fi/NVS product contracts
- Templates: plan-template.md ✅ no change; spec-template.md ✅; tasks-template.md ✅
- Docs updated: AGENTS.md (pending); README.md (pending)
- Feature 001 superseded by 002-diponto-text
-->

# esp32_led_painel Constitution

## Core Principles

### I. Hardware Contracts Are Stable

GPIO pinout, electrical function, bootstrapping constraints, bus frequency, partition layout, NVS schema, OTA strategy, TLS configuration, MQTT topics, payload schemas, QoS, retain, LWT and keepalive, codec selection, I2S pinout, board config and audio pipeline topology (when applicable) are compatibility contracts. They MUST NOT change without explicit requirement in `spec.md`, `plan.md` and `tasks.md`, documented rationale, migration impact and validation plan.

HUB75 display contracts for **Milestone 0**:

- **Board**: Adafruit MatrixPortal S3 (`esp32s3`) with factory HUB75 harness — no custom GPIO pin map in this repository; pin assignment is the board preset (`esphome/esp-hub75` MatrixPortal profile)
- **Panel topology**: 1× HUB75 module, **128×64** pixels, scan **1/32**, interface **HUB75E** (pin E via preset; e.g. SXHL-P2.5-1735 class panels)
- **Display driver**: `esphome/esp-hub75` with MatrixPortal preset, DMA GDMA on ESP32-S3
- **Shift driver**: **`Hub75ShiftDriver::FM6126A`** — fixed for Milestone 0; changes require spec/plan amendment
- **Output clock**: **10 MHz** initial default (`sdkconfig.defaults.esp32s3`); changes require spec/plan amendment
- **PSRAM**: quad mode (`CONFIG_SPIRAM_MODE_QUAD`); octal MUST NOT be enabled (conflicts with HUB75 GPIO on MatrixPortal)

LVGL, Wi-Fi stack pin usage, NVS product schema and MQTT are **not** active contracts until a future approved feature reintroduces them.

### II. Spec First, Code Second

Feature work follows the Spec Kit flow: constitution, specify, clarify when useful, checklist, plan, tasks and implement. Specification, planning and task-generation phases MUST NOT implement code. Implementation MUST follow approved `spec.md`, `plan.md` and `tasks.md`; if implementation reveals a requirement, architecture or hardware-contract change, update the Spec Kit artifacts before continuing.

### III. Field Diagnosability Is Required

Firmware MUST produce actionable diagnostics for real hardware and field support.

**Milestone 0 minimum logs** (tags `main`, `hub75_drv`, `FM6126A`, `GdmaDma`):

- Panel virtual dimensions after init
- FM6126A shift-driver init success or failure
- HUB75 `begin()` failure reason
- Text draw completion (`diponto_text` or equivalent)

Structured diagnostic enums, MQTT payloads and app/backend contracts are deferred until a connectivity feature is specified.

### IV. Embedded Clean Architecture, Pragmatically Applied

**Milestone 0** allows the smallest structure that works:

- `components/hub75_driver/` — HUB75 hardware access (preset pins, FM6126A, fill/draw_pixels)
- `main/` — boot, brightness, static text rendering

Text rendering MUST use a minimal bitmap font drawn through the hub75 driver API. **MUST NOT** introduce LVGL, Wi-Fi, NVS product storage, MQTT, or multi-component app layers without an approved spec.

When scope grows, prefer adding boundaries only when a second subsystem (e.g. connectivity) is specified — YAGNI until then.

### V. Validation Before Done

Every code change MUST be validated with the strongest practical check available.

- `idf.py -B ~/esp32_led_painel-build build` is **mandatory** for all firmware changes
- `idf.py size` is optional for Milestone 0 minimal firmware
- `idf.py flash`, `idf.py erase_flash`, destructive NVS operations and destructive operations on real hardware MUST NOT run without explicit user authorization
- **Milestone 0 hardware done criterion**: the string **DIPONTO** is legible on the physical panel at ~1 m viewing distance

## Project Constraints

This repository is **esp32_led_painel**, firmware for a HUB75 LED panel on Adafruit MatrixPortal S3.

| Constraint | Value |
|------------|-------|
| Framework | ESP-IDF v6.1 (local dev), target **esp32s3** |
| ESP-ADF | Not used |
| Board | Adafruit MatrixPortal S3 + factory HUB75 harness |
| Display | 1× HUB75 **128×64**, driver `esphome/esp-hub75` (MatrixPortal preset) |
| Shift driver | **FM6126A** |
| UI (Milestone 0) | Static bitmap text **DIPONTO** via HUB75 (no LVGL) |
| Active milestone | Show **DIPONTO** on panel at boot |
| Connectivity | None (Milestone 0) |
| Build gate | `idf.py -B ~/esp32_led_painel-build build` |
| Flash policy | No flash/erase without explicit user authorization |

Secrets, credentials, private keys, private certificates and production tokens MUST NEVER be committed to the repository or feature specs.

### Milestone 0 — DIPONTO splash

On boot, after HUB75 init with FM6126A:

1. Clear display (black background)
2. Draw **DIPONTO** centered on the 128×64 canvas in a high-contrast color (default: white on black)
3. Hold indefinitely (no animation required)

### Deferred (explicitly out of scope until new spec)

- LVGL and dashboard UI (clock, weather)
- Wi-Fi, SNTP, HTTP, Open-Meteo
- NVS product configuration and weather cache
- MQTT, OTA, BLE/Blufi
- Multi-profile display bring-up debug firmware
- Custom partition table and OTA strategy
- Field diagnostics enum and app/backend MQTT contract

## Cross-Platform Development

The template supports Windows, Linux and macOS development. Spec Kit scripts are versioned in both `.specify/scripts/bash/` and `.specify/scripts/powershell/`.

- Use Bash scripts on Linux, macOS, WSL or Git Bash.
- Use PowerShell scripts on native Windows.
- Keep generated feature specs and tasks portable: avoid OS-specific paths unless the feature explicitly targets that OS.

## Governance

This constitution supersedes informal project habits and applies to this project until explicitly amended. Amendments REQUIRE a documented reason, affected compatibility contracts, migration impact and validation plan. `AGENTS.md` may add operational guidance, but it MUST NOT weaken these principles.

Feature `001-lvgl-clock-weather` is **superseded** by `002-diponto-text` as of 2026-06-19; historical artifacts remain for reference only.

**Version**: 2.0.0 | **Ratified**: 2026-06-14 | **Last Amended**: 2026-06-19
