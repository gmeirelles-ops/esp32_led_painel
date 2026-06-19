# Implementation Plan: Dashboard LVGL — Horário e Clima

**Branch**: `001-lvgl-clock-weather` | **Date**: 2026-06-18 (updated 2026-06-18 — display bring-up debug) | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-lvgl-clock-weather/spec.md` + field issue **painel totalmente preto** (serial OK, sem imagem)

**Note**: Filled by `/speckit-plan`. Design artifacts: [research.md](./research.md), [data-model.md](./data-model.md), [quickstart.md](./quickstart.md), [contracts/](./contracts/).

## Summary

Firmware ESP-IDF para Adafruit MatrixPortal S3 exibe dashboard LVGL em painel HUB75 **128×64**: tela de **horário** (SNTP + fuso em NVS) e tela de **clima** (Open-Meteo + cidade/coordenadas em NVS), alternando automaticamente a cada **10 s**. Driver HUB75 via preset **`adafruit-matrix-portal-s3`** do componente **`esphome-libs/esp-hub75`** — sem mapa GPIO customizado no repositório. Arquitetura em componentes ESP-IDF alinhada à constituição: `drivers`, `display_lvgl`, `app/scenes`, `connectivity`, `storage`, `protocol`.

**Extension (2026-06-18)**: Programa de diagnóstico **painel preto** — teste completo em camadas (serial → elétrico → HUB75 bare-metal → LVGL smoke → app UI → matriz sdkconfig) documentado em [contracts/display-bringup-debug.md](./contracts/display-bringup-debug.md) e [quickstart.md](./quickstart.md) §9. Implementação via Kconfig `CONFIG_PAINEL_DISPLAY_BRINGUP` na fase `/speckit-tasks` / `/speckit-implement`; **não altera pinagem GPIO**.

## Technical Context

**Language/Version**: C/C++17, ESP-IDF **v6.1**, target **esp32s3**

**Primary Dependencies**:
- `esphome-libs/esp-hub75` (`components/hub75`) — preset `adafruit-matrix-portal-s3`, DMA GDMA
- `lvgl/lvgl` (Component Registry) + `espressif/esp_lvgl_port` — UI e task LVGL
- ESP-IDF: `esp_wifi`, `esp_netif`, `esp_http_client`, `esp_sntp`, `nvs_flash`

**Storage**: NVS flash — namespace `painel_cfg` (cidade, coordenadas, fuso, cache clima, versão schema); namespace `wifi` (SSID/senha para marco inicial)

**Testing**: Unity/host tests para parser Open-Meteo e mapeamento WMO→PT em `test/protocol/`; build gate `idf.py build`; validação em hardware real conforme [quickstart.md](./quickstart.md)

**Target Platform**: Adafruit MatrixPortal S3 (ESP32-S3, 8 MB flash, PSRAM), 1× painel HUB75 128×64

**Project Type**: Firmware embarcado monolítico (componentes ESP-IDF)

**Performance Goals**:
- Refresh LVGL ≥ 10 FPS perceptível; painel HUB75 `min_refresh_rate` 60 Hz (driver)
- Alternância de cena estável em 10 s ± 200 ms
- Boot até hora legível ≤ 2 min (spec SC-001) com Wi-Fi estável

**Constraints**:
- Sem pinagem GPIO customizada; contrato = preset MatrixPortal + `Hub75Config` documentado em [research.md](./research.md)
- LVGL heap preferencialmente em PSRAM; monitorar heap interno livre ≥ 32 KB após init
- HTTP Open-Meteo sem TLS custom (HTTPS padrão ESP-IDF CA bundle)
- Clima: fetch a cada **30 min** online; cache NVS offline
- Build fora do repo por path não-ASCII: `idf.py -B ~/esp32_led_painel-build build`

**Scale/Scope**: 2 telas LVGL, 1 dispositivo, sem MQTT/OTA/BLE neste marco

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Princípio | Avaliação (pré-design) | Avaliação (pós-design) |
|-----------|--------------------------|---------------------------|
| **I. Hardware contracts** | PASS — preset `adafruit-matrix-portal-s3`; `Hub75Config` (128×64, shift driver, brightness) fixado em research.md como contrato; LVGL (RGB565, heap, fontes) fixado no plano | PASS — contratos em [contracts/hub75-display.md](./contracts/hub75-display.md) e research.md |
| **II. Spec first** | PASS — plano deriva de spec.md aprovada; sem código de aplicação nesta fase | PASS |
| **III. Field diagnosability** | PASS — enum `painel_diag_code_t` em [contracts/diagnostics.md](./contracts/diagnostics.md) | PASS |
| **IV. Clean architecture** | PASS — boundaries por componente definidos abaixo | PASS — protocol/storage/connectivity separados |
| **V. Validation** | PASS — `idf.py build` + `idf.py size` obrigatórios; quickstart para hardware | PASS |

**Gate result**: PASS — prosseguir para implementação via `/speckit-tasks`.

### Constitution re-check — display bring-up extension

| Princípio | Avaliação |
|-----------|-----------|
| **I. Hardware contracts** | PASS — GPIO preset unchanged; sdkconfig matrix M1–M7 only adjusts shift driver, wiring scan, layout grid, clock, bit depth (allowed per hub75-display.md) |
| **III. Field diagnosability** | PASS — tiered tests + structured log `display_bringup: step=N` |
| **V. Validation** | PASS — each tier has explicit pass/fail; hardware validation mandatory before closing black-panel issue |

## Phase 2: Display Black Panel Diagnostic Program

**Trigger**: Panel fully black while serial shows `hub75_drv`, `GdmaDma`, `display_lvgl`, `app_ctrl` healthy.

**Goal**: Identify failure layer in ≤ 6 test sessions on hardware.

### Hypothesis ranking (pre-hardware)

| Rank | Hypothesis | Tier that confirms/refutes |
|------|------------|----------------------------|
| H1 | LVGL flush never paints visible pixels (stride/area/timing) | T2 pass, T3 fail |
| H2 | HUB75 scan/topology mismatch (128×64 vs 2×2 64×32, wrong shift chip) | T2 fail → T5 M2–M5 |
| H3 | Framebuffer stays zero; DMA scans black | T2 fail on full red fill |
| H4 | Brightness/OE too low or power marginal | T2.5 / M1 / Tier 1 |
| H5 | App UI hidden or never invalidated; splash skipped | T3 pass, T4 fail |
| H6 | Theme: white-on-black text too small/dim at 1 m | T4 with large bring-up label |

### Implementation outline (for tasks.md)

1. **Kconfig** (`main/Kconfig.projbuild`):
   - `CONFIG_PAINEL_DISPLAY_BRINGUP` — run Tier 2 sequence at boot, skip Wi-Fi/app or delay app until sequence done
   - `CONFIG_PAINEL_DISPLAY_BRINGUP_LVGL` — after Tier 2, run Tier 3 LVGL smoke instead of dashboard
   - `CONFIG_PAINEL_DISPLAY_BRINGUP_HOLD_MS` — default 3000 ms per color step

2. **`hub75_driver`**: expose `hub75_driver_fill_rgb()` + `hub75_driver_run_bringup_sequence()` wrapping `Hub75Driver::fill()`.

3. **`main/esp32_led_painel.c`**: conditional boot path:
   ```text
   hub75_init → [bringup sequence] → [optional lvgl smoke] → normal init
   ```

4. **No change** to Wi-Fi/NVS/MQTT contracts during debug builds.

5. **Lab matrix**: document results in quickstart §9 table; promote winning sdkconfig variant to `sdkconfig.defaults.esp32s3` only after hardware PASS on T4.

### Exit criteria

- Tier 2.1 (full red) PASS **and** Tier 4.1 splash or clock visible, **or**
- Documented hardware fault (power/cable/dead module) with Tier 0–1 evidence.

## Project Structure

### Documentation (this feature)

```text
specs/001-lvgl-clock-weather/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── hub75-display.md
│   ├── display-bringup-debug.md   # black panel tiered tests
│   ├── nvs-schema.md
│   ├── open-meteo-api.md
│   └── diagnostics.md
├── checklists/
│   └── requirements.md
└── tasks.md             # /speckit-tasks (próximo passo)
```

### Source Code (repository root)

```text
CMakeLists.txt
sdkconfig.defaults
sdkconfig.defaults.esp32s3
main/
├── CMakeLists.txt
├── idf_component.yml          # deps: hub75 wrapper, app, connectivity...
└── esp32_led_painel.c           # app_main: init sequencial, delega app_controller

components/
├── hub75_driver/                # drivers — esp-hub75 preset MatrixPortal, init/start
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   ├── include/hub75_driver.h
│   └── hub75_driver.c
├── display_lvgl/                # LVGL port, flush → hub75 framebuffer, lvgl task
│   ├── CMakeLists.txt
│   ├── include/display_lvgl.h
│   └── display_lvgl.c
├── storage/                     # NVS: painel_cfg, weather cache, wifi creds
│   ├── CMakeLists.txt
│   ├── include/painel_storage.h
│   ├── painel_storage.c
│   └── painel_nvs_schema.h      # espelha contracts/nvs-schema.md
├── connectivity/                # Wi-Fi STA, SNTP, HTTP fetch orchestration
│   ├── CMakeLists.txt
│   ├── include/connectivity.h
│   ├── wifi_manager.c
│   ├── sntp_service.c
│   └── http_client.c
├── protocol/                    # Open-Meteo JSON parse, geocoding, WMO→PT
│   ├── CMakeLists.txt
│   ├── include/open_meteo.h
│   ├── open_meteo_forecast.c
│   ├── open_meteo_geocode.c
│   └── wmo_condition_pt.c
└── app/                         # domain + scenes + orchestration
    ├── CMakeLists.txt
    ├── include/
    │   ├── app_controller.h
    │   ├── dashboard_state.h
    │   ├── scene_clock.h
    │   └── scene_weather.h
    ├── app_controller.c
    ├── dashboard_state.c
    ├── scene_clock.c
    └── scene_weather.c

test/
└── protocol/
    ├── CMakeLists.txt
    ├── test_open_meteo_parse.c
    └── test_wmo_pt.c
```

**Structure Decision**: Monorepo ESP-IDF com um componente por boundary da constituição. `main/` apenas bootstrap; lógica de produto em `components/app/`; parsing HTTP em `protocol/`; transporte em `connectivity/`; persistência em `storage/`. Testes unitários isolados em `test/protocol/` (sem hardware).

### Runtime tasks (FreeRTOS)

| Task / timer | Prioridade | Stack (mín.) | Responsabilidade |
|--------------|------------|--------------|------------------|
| `lvgl_task` | 4 | 4096 | `lv_timer_handler()`, mutex display |
| `app_task` | 5 | 4096 | alternância cenas, subscribe eventos Wi-Fi/SNTP/clima |
| `wifi`/event loop | default IDF | — | `esp_event` handlers leves → fila app |
| `esp_timer` weather | — | — | fetch clima 30 min |
| `esp_timer` clock UI | — | — | refresh label hora 1 s |

## Hardware & Display Contract (summary)

Detalhes completos em [contracts/hub75-display.md](./contracts/hub75-display.md) e [research.md](./research.md).

| Parâmetro | Valor contratual |
|-----------|------------------|
| Board preset | `adafruit-matrix-portal-s3` |
| Canvas lógico LVGL | 128 × 64 px |
| Panel config | `panel_width=128`, `panel_height=64` (módulo único 128×64; validar scan driver em hardware) |
| Pixel format LVGL → HUB75 | RGB565 |
| LVGL color depth | 16 bit |
| LVGL heap | 48 KB (PSRAM preferido via Kconfig) |
| Fontes | Montserrat **24** (hora/temp), **12** (cidade/condição/erro) — built-in LVGL |
| Brightness default | 128/255 |
| `double_buffer` hub75 | false (LVGL gerencia buffer) |
| Rotação | 0° |

### Screen layouts (128×64)

**Clock scene**
- Centro: `HH:MM` (font 24)
- Rodapé opcional: `DD/MM` (font 12) se couber sem clipping
- Erro: `"Sem hora"` ou `"Sem Wi-Fi"` (font 12, centralizado)

**Weather scene**
- Topo: nome cidade truncado 12 chars (font 12)
- Centro: `"25°C"` arredondado inteiro (font 24)
- Rodapé: condição PT ≤ 12 chars, ex. `"Nublado"` (font 12)
- Cache stale: prefixo `"~"` na temperatura ou sufixo `"*"` na condição
- Erro: `"Sem clima"`, `"Cidade?"`, `"Sem Wi-Fi"`

**Splash / init**
- `"Iniciando..."` até display + primeira cena prontos

## NVS & External API (summary)

- Schema v1: [contracts/nvs-schema.md](./contracts/nvs-schema.md)
- Open-Meteo: [contracts/open-meteo-api.md](./contracts/open-meteo-api.md)
- Diagnósticos: [contracts/diagnostics.md](./contracts/diagnostics.md)

## Wi-Fi provisioning (marco inicial)

- Credenciais em namespace NVS `wifi` (`ssid`, `pass` — blob/string com tamanho máximo documentado)
- Desenvolvimento: fallback `CONFIG_PAINEL_WIFI_SSID` / `CONFIG_PAINEL_WIFI_PASSWORD` via Kconfig quando NVS vazio
- BLE/Blufi, captive portal e app Flutter: **fora de escopo** (spec FR-013)

## Initialization sequence

```text
nvs_flash_init → storage_load_config → hub75_driver_init → display_lvgl_init
→ wifi_manager_start → [on GOT_IP] sntp_service_start → app_controller_start
→ geocode if lat/lon missing → weather_fetch → scene loop
```

## Complexity Tracking

> Nenhuma violação da constituição que exija exceção. Estrutura de 6 componentes segue boundaries recomendados; alternativa monolítica em `main/` foi rejeitada por testabilidade e diagnóstico.
