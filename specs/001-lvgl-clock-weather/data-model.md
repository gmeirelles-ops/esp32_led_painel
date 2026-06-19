# Data Model: Dashboard LVGL — Horário e Clima

**Feature**: `001-lvgl-clock-weather` | **Date**: 2026-06-18

## Entity relationship

```text
PainelConfig (NVS) ──► GeoQuery ──► Open-Meteo APIs
        │                    │
        ▼                    ▼
  WeatherCache (NVS)   WeatherSnapshot (RAM)
        │                    │
        └──────────► DashboardState (RAM) ◄── ClockState (RAM)
                              │
                              ▼
                        SceneView (LVGL)
```

---

## PainelConfig

Configuração persistente do dispositivo. Owner: `storage/`.

| Field | Type | NVS key | Constraints | Default |
|-------|------|---------|-------------|---------|
| schema_version | uint8 | `cfg_ver` | Must be `1` | `1` |
| city | string | `city` | 1–48 chars UTF-8, trimmed | empty |
| latitude | float | `lat` | −90..90, 0 = unset | `0` |
| longitude | float | `lon` | −180..180, 0 = unset | `0` |
| timezone | string | `tz` | IANA, 1–32 chars | `America/Sao_Paulo` |
| scene_dwell_ms | uint32 | `dwell_ms` | 3000–60000 | `10000` |

**Validation rules**:
- Clima exige `(lat, lon)` válidos OU `city` não vazia (geocode on demand).
- Se `city` vazia e coords zero → weather state `CONFIG_MISSING`.
- Alteração de `cfg_ver` exige migração documentada; v1 inicial.

**State transitions (config load)**:
```text
NVS_MISSING → defaults applied → CONFIG_LOADED
NVS_CORRUPT  → log PAINEL_DIAG_NVS_ERROR → defaults + CONFIG_DEGRADED
```

---

## WifiCredentials

Owner: `storage/` / `connectivity/`. Namespace separado `wifi`.

| Field | Type | NVS key | Constraints |
|-------|------|---------|-------------|
| ssid | string | `ssid` | 1–32 chars |
| password | string | `pass` | 0–64 chars |

**Note**: Senha nunca logada. Fallback Kconfig só quando ambas chaves ausentes.

---

## ClockState

Runtime only. Owner: `app/` + `connectivity/sntp_service`.

| Field | Type | Description |
|-------|------|-------------|
| sync_status | enum | `PENDING`, `SYNCED`, `ERROR` |
| last_sync_unix | int64 | UTC epoch of last SNTP sync |
| last_error | painel_diag_code_t | SNTP/Wi-Fi cause |
| display_hhmm | char[6] | Cached "HH:MM" for UI |
| display_date | char[6] | Cached "DD/MM" optional |

**Transitions**:
```text
PENDING ──[SNTP sync OK]──► SYNCED
PENDING ──[timeout/no Wi-Fi]──► ERROR
SYNCED ──[Wi-Fi lost]──► PENDING (keep last time optional: show stale with '*')
```

---

## WeatherSnapshot

Runtime view model. Owner: `app/`; populated by `protocol/` + `storage/`.

| Field | Type | Description |
|-------|------|-------------|
| temp_c | float | Current °C from API |
| temp_display | char[8] | e.g. "25°C" |
| wmo_code | int | Raw WMO code |
| condition_pt | char[13] | Short PT label |
| fetched_unix | int64 | When data was obtained |
| source | enum | `LIVE`, `CACHE`, `NONE` |
| fetch_status | enum | `OK`, `PENDING`, `ERROR`, `CONFIG_MISSING` |
| last_error | painel_diag_code_t | HTTP/parse/geo error |

**Stale rule**: `source=CACHE` when `now - fetched_unix > 1800` (30 min) but still displayed with stale indicator.

---

## WeatherCache

Persistent subset for offline display. Owner: `storage/`. Keys in `painel_cfg`:

| Field | Type | NVS key |
|-------|------|---------|
| temp_c_x10 | int16 | `wx_t_x10` |
| wmo_code | int16 | `wx_code` |
| fetched_unix | int64 | `wx_ts` |

On successful live fetch, cache updated atomically (write temp/code/ts, then commit).

---

## DashboardState

Orchestration state. Owner: `app/app_controller`.

| Field | Type | Description |
|-------|------|-------------|
| active_scene | enum | `INIT`, `CLOCK`, `WEATHER` |
| connectivity | enum | `OFFLINE`, `CONNECTING`, `ONLINE` |
| last_scene_switch_ms | uint32 | For dwell timing |
| init_complete | bool | Splash dismissed |

**Scene rotation**:
```text
INIT ──[display ready]──► CLOCK ◄──10s──► WEATHER ──10s──► ...
```

Errors on inactive scene still advance rotation (spec US3 scenario 2).

---

## ConnectivityState

Owner: `connectivity/wifi_manager`.

| Field | Type | Description |
|-------|------|-------------|
| wifi_status | enum | `DISCONNECTED`, `CONNECTING`, `CONNECTED` |
| retry_count | uint8 | Reconnect attempts |
| rssi | int8 | Last RSSI when connected |

Events pushed to `app_task` queue: `WIFI_UP`, `WIFI_DOWN`, `SNTP_SYNC`, `WEATHER_READY`, `WEATHER_FAIL`.

---

## Diagnostic codes (summary)

Full enum: [contracts/diagnostics.md](./contracts/diagnostics.md).

| Code | Meaning |
|------|---------|
| `PAINEL_DIAG_OK` | Success |
| `PAINEL_DIAG_WIFI_NO_CRED` | No SSID configured |
| `PAINEL_DIAG_WIFI_AUTH_FAIL` | Authentication failed |
| `PAINEL_DIAG_WIFI_NO_AP` | AP not found |
| `PAINEL_DIAG_SNTP_FAIL` | Time sync failed |
| `PAINEL_DIAG_HTTP_TIMEOUT` | Weather HTTP timeout |
| `PAINEL_DIAG_HTTP_PARSE` | JSON parse error |
| `PAINEL_DIAG_GEO_NOT_FOUND` | City not found |
| `PAINEL_DIAG_NVS_ERROR` | NVS read/write failure |
| `PAINEL_DIAG_DISPLAY_INIT` | HUB75/LVGL init failure |
| `PAINEL_DIAG_DISPLAY_BRINGUP_FAIL` | Bare-metal fill/pattern step failed (debug) |
| `PAINEL_DIAG_DISPLAY_FLUSH_FAIL` | LVGL smoke flush produced no visible change (debug) |

---

## DisplayBringupSession (debug builds only)

Runtime/lab entity for black-panel isolation. Owner: `hub75_driver` / `display_lvgl` when `CONFIG_PAINEL_DISPLAY_BRINGUP`. Not persisted to NVS.

| Field | Type | Description |
|-------|------|-------------|
| tier | uint8 | Current tier 0–5 per [display-bringup-debug.md](./contracts/display-bringup-debug.md) |
| step | uint8 | Step within tier (e.g. 2.1 → step=1) |
| last_result | enum | `PASS`, `FAIL`, `SKIP` |
| matrix_variant | char[4] | sdkconfig matrix ID: `M0`…`M7` |
| last_error | painel_diag_code_t | Bring-up or flush failure |

**Transitions**:
```text
IDLE ──[boot + Kconfig bringup]──► TIER2_RUNNING
TIER2_RUNNING ──[step PASS]──► next step | TIER3 if all PASS
TIER2_RUNNING ──[step FAIL]──► TIER5_MATRIX (manual sdkconfig) or ABORT
TIER3_RUNNING ──[smoke PASS]──► NORMAL_APP
TIER3_RUNNING ──[smoke FAIL]──► FLUSH_DEBUG (display_lvgl)
```
