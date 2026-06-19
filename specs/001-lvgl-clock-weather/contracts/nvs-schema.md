# Contract: NVS Schema

**Version**: 1 | **Feature**: `001-lvgl-clock-weather`

## Namespaces

| Namespace | Purpose | Owner component |
|-----------|---------|-----------------|
| `painel_cfg` | Product config + weather cache | `storage` |
| `wifi` | STA credentials (marco inicial) | `storage` / `connectivity` |

## Namespace `painel_cfg`

| Key | Type | Max size | Required | Description |
|-----|------|----------|----------|-------------|
| `cfg_ver` | u8 | 1 | yes | Schema version; current `1` |
| `city` | string | 48 | no | Display name, UTF-8 |
| `lat` | blob/float | 4 | no | Latitude; `0.0` = unset |
| `lon` | blob/float | 4 | no | Longitude; `0.0` = unset |
| `tz` | string | 32 | yes | IANA timezone |
| `dwell_ms` | u32 | 4 | no | Scene dwell; default 10000 |
| `wx_t_x10` | i16 | 2 | no | Cached temp × 10 |
| `wx_code` | i16 | 2 | no | Cached WMO code |
| `wx_ts` | i64 | 8 | no | Cache timestamp Unix UTC |

### Compatibility rules

- Devices without `cfg_ver` treated as v0 → migrate to v1 defaults on first boot.
- New keys additive only within v1; breaking changes require `cfg_ver=2` + migration task.
- Never delete `city`/`lat`/`lon` on fetch failure.

## Namespace `wifi`

| Key | Type | Max size | Required | Description |
|-----|------|----------|-------------|
| `ssid` | string | 32 | yes* | Wi-Fi SSID |
| `pass` | string | 64 | no | Wi-Fi password |

\*Required for production; dev may use Kconfig when namespace empty.

## Provisioning (out of UX scope)

Initial write via one of (defined in tasks):

1. `idf.py monitor` + documented NVS CLI / test helper
2. Kconfig defaults for lab (`CONFIG_PAINEL_WIFI_*`, `CONFIG_PAINEL_DEFAULT_CITY`)

Example logical record after provisioning:

```json
{
  "cfg_ver": 1,
  "city": "São Paulo",
  "lat": -23.5505,
  "lon": -46.6333,
  "tz": "America/Sao_Paulo",
  "dwell_ms": 10000
}
```

(NVS stores typed keys, not JSON blob.)

## Secrets policy

- Wi-Fi password and any tokens MUST NOT appear in logs or specs committed with real values.
- No API key for Open-Meteo in v1.
