# Data Model: Dashboard LVGL Dinâmico

## scene_layout_profile

Derivado em runtime de `display_lvgl_hor_res()` / `display_lvgl_ver_res()` (build-time resolution).

| Field | Type | compact (≤128w) | large (>128w) |
|-------|------|-----------------|---------------|
| tier | enum | `SCENE_LAYOUT_COMPACT` | `SCENE_LAYOUT_LARGE` |
| font_primary | LVGL font | Montserrat 24 | Montserrat 48 |
| font_secondary | LVGL font | Montserrat 12 | Montserrat 24 |
| pad_outer | int16 | 2 | 4 |
| time_y_offset | int16 | -6 | -12 |
| date_y_offset | int16 | -2 | -4 |

## clock_presentation_state

| State | Enum | UI rule |
|-------|------|---------|
| pending | `CLOCK_SYNC_PENDING` | Mostrar "Sincronizando…"; ocultar hora/data |
| synced | `CLOCK_SYNC_SYNCED` | HH:MM + DD/MM se `tm_year > 2020` |
| error | `CLOCK_SYNC_ERROR` | "Sem hora" ou "Sem Wi-Fi" conforme `wifi` flag |

**Validation**: Nunca exibir `s_time`/`s_date` vazios sem mensagem de erro visível.

## weather_presentation_state

| Field | Source | Visual |
|-------|--------|--------|
| live | `error=false, stale=false` | temp normal, condição normal |
| stale | `stale=true` | prefixo `~`, opa reduzida |
| error | `error=true` | mensagem central, temp/cond vazios |

## transition_state

| Field | Type | Notes |
|-------|------|-------|
| from_scene | `dash_scene_t` | clock ou weather |
| to_scene | `dash_scene_t` | |
| anim_active | bool | bloqueia reentrada |
| duration_ms | uint16 | 250 + 250 default |

## connectivity_bootstrap (one-shot)

Executado em `app_controller_start`:

1. Register callback
2. If Wi-Fi connected → enqueue or inline WIFI_UP handler
3. If SNTP already synced → set CLOCK_SYNC_SYNCED

Não persistido em NVS.

## Relationships

- `app_controller` owns transition + bootstrap
- `scene_layout` read-only helper usado por `scene_clock`, `scene_weather`
- `painel_tz_apply` em storage; chamado antes de exibir hora local
