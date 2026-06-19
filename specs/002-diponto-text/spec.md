# Feature Specification: DIPONTO Text Splash

**Feature Branch**: `002-diponto-text`

**Created**: 2026-06-19

**Status**: Active — **hardware validated** (DIPONTO visible on SXHL-P2.5-1735, 2026-06-19)

**Input**: Milestone 0 — reset firmware; show **DIPONTO** on HUB75 128×64 via FM6126A, no LVGL/Wi-Fi.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Ver "DIPONTO" ao ligar (Priority: P1)

Como operador, quero ver a palavra **DIPONTO** centralizada no painel ao energizar o dispositivo, para confirmar que o driver HUB75 e o painel funcionam.

**Acceptance Scenarios**:

1. **Given** MatrixPortal S3 + painel 128×64 (SXHL-P2.5-1735 class) conectados, **When** o firmware inicia, **Then** o painel exibe **DIPONTO** legível a ~1 m (texto claro sobre fundo escuro).
2. **Given** boot concluído, **When** observo por 30 s, **Then** o texto permanece estável (sem reset, sem flicker anormal).
3. **Given** monitor serial, **When** boot, **Then** logs mostram FM6126A init OK e dimensões 128×64.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Firmware MUST initialize HUB75 with MatrixPortal preset and **FM6126A** shift driver.
- **FR-002**: Firmware MUST draw the string **DIPONTO** centered on the 128×64 canvas.
- **FR-003**: Firmware MUST NOT depend on LVGL, Wi-Fi, NVS product schema, or HTTP.
- **FR-004**: Default colors: white text on black background; brightness maximum practical (255).

### Non-Functional Requirements

- **NFR-001**: `idf.py build` succeeds for target `esp32s3`.
- **NFR-002**: Boot to visible text within 5 s on hardware.

## Success Criteria

- **SC-001**: "DIPONTO" legível no painel físico.
- **SC-002**: Build gate green without LVGL in dependency tree.

## Out of Scope

- LVGL, clock, weather, NVS, MQTT, OTA, animation, scrolling text, touch/buttons.
