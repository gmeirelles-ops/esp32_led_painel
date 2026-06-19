# Contract: Diagnostics — Dynamic Dashboard

**Version**: 1.0 | **Feature**: `005-lvgl-dynamic-dashboard`

## Boot / connectivity

```
I storage: timezone BRT3 (from America/Sao_Paulo)
I app_ctrl: connectivity bootstrap wifi=1 sntp_synced=0
I sntp_svc: time synced
I sntp_svc: local time HH:MM        # hora apenas, sem data completa em log
I app_ctrl: clock sync -> SYNCED
```

## Clock regression guard

If `CLOCK_SYNC_SYNCED` but formatted time empty:

```
W scene_clock: synced but invalid tm (year=1970)
```

## Scene transitions

```
I app_ctrl: scene transition clock -> weather
```

## Error states (UI must match)

| Condition | Display |
|-----------|---------|
| Wi-Fi down | "Sem Wi-Fi" |
| SNTP pending | "Sincronizando…" |
| SNTP failed | "Sem hora" |
| Weather error | "Sem clima" |
| Weather stale | `~NN°C` |

## Hardware validation

- [ ] Single 128×64: hora em ≤2 min após boot com Wi-Fi
- [ ] 2×2 256×128: fontes large legíveis a 1 m
- [ ] 4+ transições em 5 min sem tela preta >500 ms
- [ ] Wi-Fi já conectado no boot: SNTP inicia (bootstrap)
