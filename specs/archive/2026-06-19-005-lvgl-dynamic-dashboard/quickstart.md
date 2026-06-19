# Quickstart: Dashboard LVGL Dinâmico

## Prerequisites

- ESP-IDF v6.1, build dir `~/esp32_led_painel-build`
- Wi-Fi creds em NVS ou Kconfig lab fallback
- Feature `004` topologia (single default)

## Build

```bash
idf.py -B ~/esp32_led_painel-build build
```

## Validar correção do relógio

1. Flash (com autorização) e monitor serial
2. Confirmar sequência:
   - `connectivity bootstrap wifi=1`
   - `sntp_svc: time synced`
   - `clock sync -> SYNCED`
3. Tela horário: `HH:MM` e data visíveis em ≤2 min
4. **Teste de regressão**: reiniciar com Wi-Fi já associado (warm boot) — hora deve sincronizar sem ficar em "Sem hora" indefinidamente

## Validar layout large (2×2)

```bash
idf.py menuconfig  # Display topology → Four panels 2×2
idf.py -B ~/esp32_led_painel-build-2x2 build
```

Verificar fontes maiores em 256×128.

## Validar transições

Observar 5 min: alternância horário↔clima a cada 10 s com fade perceptível.

## Related

- [ui-layout.md](./contracts/ui-layout.md)
- [scene-transitions.md](./contracts/scene-transitions.md)
- [diagnostics.md](./contracts/diagnostics.md)
- [research.md](./research.md) — causa raiz hora vazia
