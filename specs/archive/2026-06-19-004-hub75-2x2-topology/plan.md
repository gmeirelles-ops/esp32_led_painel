# Implementation Plan: HUB75 Display Topology (1× vs 2×2)

**Branch**: `004-hub75-2x2-topology` | **Date**: 2026-06-19 | **Spec**: [spec.md](./spec.md)

## Summary

Adicionar `choice PAINEL_TOPOLOGY` em `main/Kconfig.projbuild` mapeando para `Hub75Config` em `hub75_board_config.h`. Perfil default single (128×64); opcional 2×2 com quatro módulos 128×64 (256×128). LVGL framebuffer dinâmico em PSRAM; cenas com dimensões do display.

## Technical Context

- **Language**: C/C++, ESP-IDF v6.1, target esp32s3
- **Dependencies**: `esphome/esp-hub75`, LVGL 9.x
- **Storage**: N/A (build-time only)
- **Testing**: `idf.py -B ~/esp32_led_painel-build build`, `idf.py size`
- **Constraints**: GPIO/preset/FM6126A/10 MHz inalterados

## Constitution Check

| Principle | Status |
|-----------|--------|
| I Hardware | AMEND — topologia build-time; default single preservado |
| II Spec first | PASS |
| III Diagnostics | PASS — logs de topologia |
| IV Clean Architecture | PASS — Kconfig em main, mapeamento em hub75_driver, UI via display_lvgl |
| V Validation | PASS — build gate |

## Project Structure

```text
main/Kconfig.projbuild          # choice PAINEL_TOPOLOGY
components/hub75_driver/hub75_board_config.h
components/display_lvgl/display_lvgl.c
components/app/scene_clock.c
components/app/scene_weather.c
sdkconfig.defaults.esp32s3
specs/004-hub75-2x2-topology/
```

## Design Decisions

See [research.md](./research.md).

## Contracts

- [hub75-display-topology.md](./contracts/hub75-display-topology.md)
- [diagnostics.md](./contracts/diagnostics.md)
