# Feature Specification: HUB75 Display Topology (1× vs 2×2)

**Feature Branch**: `004-hub75-2x2-topology`

**Created**: 2026-06-19

**Status**: Active

**Input**: Configurar via sdkconfig/menuconfig se o firmware usa 1 painel 128×64 ou grade 2×2 com quatro módulos 128×64 (256×128 lógico).

## Constitution Amendment

A constituição v2.1.0 fixa topologia **1× 128×64**. Esta feature amplia o contrato de hardware para topologia **build-time** configurável:

- **Default (inalterado)**: 1 painel, 128×64 lógico
- **Opcional**: 4 painéis 128×64 em grade 2×2, 256×128 lógico
- **Preservado**: pinagem MatrixPortal preset, FM6126A, clock 10 MHz, PSRAM quad

## User Scenarios & Testing

### User Story 1 - Build padrão single panel (Priority: P1)

Como desenvolvedor, quero que o firmware compile e rode com 1 painel 128×64 sem alterar o comportamento validado.

**Acceptance Scenarios**:

1. **Given** `CONFIG_PAINEL_TOPOLOGY_SINGLE=y` (default), **When** build e boot, **Then** resolução virtual 128×64 e dashboard legível como antes.
2. **Given** monitor serial, **When** boot, **Then** log `hub75_drv` mostra perfil `single`, layout 1×1, virtual 128×64.

### User Story 2 - Build grade 2×2 (Priority: P1)

Como operador com 4 módulos HUB75 128×64 em grade 2×2, quero selecionar a topologia no menuconfig e ver o canvas 256×128.

**Acceptance Scenarios**:

1. **Given** `CONFIG_PAINEL_TOPOLOGY_2X2=y` e 4 painéis cabeados em cadeia, **When** boot, **Then** resolução virtual 256×128 e conteúdo LVGL preenche a tela.
2. **Given** monitor serial, **When** boot, **Then** log mostra perfil `2x2`, panel 128×64, layout 2×2, virtual 256×128.

## Requirements

### Functional Requirements

- **FR-001**: Escolha de topologia em `menuconfig` (menu Painel LED), sem editar manualmente `HUB75_LAYOUT_*` do componente.
- **FR-002**: Perfil **Single** (default): 1 painel 128×64, layout 1×1, tipo HORIZONTAL.
- **FR-003**: Perfil **2×2**: 4 painéis 128×64, layout 2×2, tipo TOP_LEFT_DOWN_ZIGZAG, virtual 256×128.
- **FR-004**: Logs de boot com perfil, dimensões por painel, layout e resolução virtual (`hub75_board`, `hub75_drv`).
- **FR-005**: LVGL framebuffer dimensionado dinamicamente conforme resolução virtual; cenas usam dimensões do display.

### Non-Functional Requirements

- **NFR-001**: Pinagem MatrixPortal inalterada; FM6126A e clock 10 MHz preservados.
- **NFR-002**: Framebuffer LVGL preferencialmente em PSRAM.
- **NFR-003**: `idf.py -B ~/esp32_led_painel-build build` obrigatório antes de concluir.

## Success Criteria

- **SC-001**: Build green com perfil Single (default).
- **SC-002**: Build green com perfil 2×2.
- **SC-003**: Single: regressão funcional 128×64; 2×2: validação em hardware com grade física correta (pendente campo).

## Out of Scope

- NVS/runtime switching de topologia
- MQTT, OTA, pinagem customizada
- Kconfig avançado para layout serpentine (v2 futura se hardware exigir)
- Redesign completo de UI para 256×128 (apenas dimensões dinâmicas + fontes existentes)
