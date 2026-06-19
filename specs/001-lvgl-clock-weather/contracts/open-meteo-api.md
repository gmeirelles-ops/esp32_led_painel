# Contract: Open-Meteo API

**Version**: 1 | **Feature**: `001-lvgl-clock-weather`

## Overview

HTTPS JSON APIs, no authentication. Owner: `protocol/` (parse), `connectivity/` (transport).

## Geocoding — resolve city to coordinates

**When**: NVS has non-empty `city` and (`lat`,`lon`) unset or zero.

```
GET https://geocoding-api.open-meteo.com/v1/search
  ?name={url_encoded_city}
  &count=1
  &language=pt
  &format=json
```

**Success** (200): use first result `latitude`, `longitude`, `name`; persist to NVS.

**Response shape** (minimal):

```json
{
  "results": [
    {
      "name": "São Paulo",
      "latitude": -23.5505,
      "longitude": -46.6333,
      "country_code": "BR"
    }
  ]
}
```

**Errors**:
- Empty `results` → `PAINEL_DIAG_GEO_NOT_FOUND`
- HTTP/timeout → `PAINEL_DIAG_HTTP_TIMEOUT`

## Forecast — current weather

**When**: Valid coordinates available; repeat every 1800 s while Wi-Fi up.

```
GET https://api.open-meteo.com/v1/forecast
  ?latitude={lat}
  &longitude={lon}
  &current=temperature_2m,weather_code
  &timezone={tz}
```

**Success** (200) — required fields:

```json
{
  "current": {
    "time": "2026-06-18T12:00",
    "temperature_2m": 25.3,
    "weather_code": 3
  }
}
```

**Processing**:
- `temperature_2m` → round to int for display `"NN°C"`
- `weather_code` → map via `wmo_condition_pt()` → ≤12 char PT string
- `current.time` → parse for `fetched_unix` cache metadata

**Errors**:
- Malformed JSON → `PAINEL_DIAG_HTTP_PARSE`
- Timeout (10 s) → retry with backoff; final `PAINEL_DIAG_HTTP_TIMEOUT`

## HTTP client settings

| Parameter | Value |
|-----------|-------|
| TLS | ESP-IDF certificate bundle |
| Timeout | 10 s |
| Max response body | 4 KB buffer |
| User-Agent | `esp32_led_painel/1.0` (optional, no secrets) |
| Retries | 3 with backoff 5 / 15 / 45 s |

## Offline behavior

- On fetch failure with valid cache → UI uses cache, `source=CACHE`, stale indicator if age > 1800 s
- On fetch failure without cache → weather scene error state

## Test fixtures

Store minimal JSON strings in `test/protocol/fixtures/` for parser unit tests (no network).
