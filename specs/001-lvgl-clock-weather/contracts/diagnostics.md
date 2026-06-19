# Contract: Field Diagnostics

**Version**: 1 | **Feature**: `001-lvgl-clock-weather`

## Enum `painel_diag_code_t`

Stable codes for logs and future app/backend integration. Header: `components/app/include/painel_diag.h` (or `protocol/` if shared).

```c
typedef enum {
    PAINEL_DIAG_OK = 0,
    PAINEL_DIAG_WIFI_NO_CRED,
    PAINEL_DIAG_WIFI_AUTH_FAILED,
    PAINEL_DIAG_WIFI_NO_AP_FOUND,
    PAINEL_DIAG_WIFI_ASSOC_FAIL,
    PAINEL_DIAG_SNTP_FAIL,
    PAINEL_DIAG_HTTP_TIMEOUT,
    PAINEL_DIAG_HTTP_TLS_FAIL,
    PAINEL_DIAG_HTTP_PARSE,
    PAINEL_DIAG_GEO_NOT_FOUND,
    PAINEL_DIAG_NVS_ERROR,
    PAINEL_DIAG_DISPLAY_INIT,
    PAINEL_DIAG_DISPLAY_BRINGUP_FAIL,
    PAINEL_DIAG_DISPLAY_FLUSH_FAIL,
    PAINEL_DIAG_LVGL_INIT,
    PAINEL_DIAG_WEATHER_CONFIG_MISSING,
    PAINEL_DIAG_INTERNAL_ERROR
} painel_diag_code_t;
```

## Logging convention

Each subsystem logs with tag + code + short message:

```
E (12345) sntp_svc: diag=5 (PAINEL_DIAG_SNTP_FAIL) sync timeout after 60s
```

**Never log**: Wi-Fi password, full HTTP response with PII beyond city name.

## UI mapping (on-screen)

| Condition | Clock scene | Weather scene |
|-----------|-------------|---------------|
| No Wi-Fi cred | — | `Sem Wi-Fi` |
| Wi-Fi down | `Sem Wi-Fi` | `Sem Wi-Fi` |
| SNTP fail | `Sem hora` | — |
| No city/coords | — | `Cidade?` |
| Geo not found | — | `Cidade?` |
| HTTP fail, no cache | — | `Sem clima` |
| HTTP fail, has cache | — | show cache + `*` stale |
| Display init fail | splash/error | splash/error |
| Display bring-up step fail | solid color wrong/missing | — (debug build only) |
| LVGL flush fail (debug) | white screen missing in smoke test | — |

## Display bring-up logging (debug builds)

When `CONFIG_PAINEL_DISPLAY_BRINGUP` enabled:

```
I (1234) display_bringup: tier=2 step=1 color=red result=PASS
E (5678) display_bringup: tier=2 step=1 color=red result=FAIL diag=13
```

Codes 13–14 map to `PAINEL_DIAG_DISPLAY_BRINGUP_FAIL` and `PAINEL_DIAG_DISPLAY_FLUSH_FAIL` respectively.

## Future MQTT (out of scope v1)

When added, payload SHOULD include `"diag": <code>` integer matching this enum without renumbering existing codes.
