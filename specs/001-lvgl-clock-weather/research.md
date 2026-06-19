# Research: Dashboard LVGL — Horário e Clima

**Feature**: `001-lvgl-clock-weather` | **Date**: 2026-06-18

## 1. Driver HUB75 e preset MatrixPortal S3

**Decision**: Usar componente ESP-IDF `esphome-libs/esp-hub75` apontando para `components/hub75`, com preset de placa `adafruit-matrix-portal-s3` (GPIO automático, sem mapa local).

**Rationale**:
- Alinhado à constituição e à spec (preset de fábrica).
- Driver DMA GDMA nativo ESP32-S3; exemplo `03_lvgl` no repositório oficial.
- ESPHome 2025.12+ usa a mesma lib — documentação madura para LVGL (`update_interval: never`, `double_buffer: false`).

**Alternatives considered**:
- `mrcodetastic/ESP32-HUB75-MatrixPanel-DMA` — maduro, mas exige pinagem manual ou wrapper; rejeitado por violar contrato “sem GPIO custom”.
- Bit-banging / driver mínimo próprio — alto risco e esforço; rejeitado.

**Implementation notes**:
- Incluir via `idf_component.yml`:
  ```yaml
  dependencies:
    esphome/esp-hub75:
      git: https://github.com/esphome-libs/esp-hub75.git
      path: components/hub75
  ```
- `Hub75Config`: preset board + `panel_width=128`, `panel_height=64`.
- Se ghosting ou scan incorreto em hardware real, ajustar **apenas** `shift_driver` (`GENERIC` vs `FM6126A`) e `scan_wiring` no wrapper `hub75_driver` — sem alterar pinos.
- Se o módulo físico for 64×32 em grade 2×2, trocar para `panel 64×32` + `layout_rows=2`, `layout_cols=2`, `TOP_LEFT_DOWN_ZIGZAG` mantendo canvas LVGL 128×64 — validação em quickstart.

---

## 2. Integração LVGL

**Decision**: `lvgl/lvgl` + `espressif/esp_lvgl_port` com flush customizado para framebuffer RGB565 do hub75; task dedicada `lvgl_task`.

**Rationale**:
- `esp_lvgl_port` integra mutex, tick e task pattern recomendado pela Espressif.
- esp-hub75 expõe buffer de pixels compatível com RGB565; LVGL 16-bit match.
- Exemplo upstream `examples/03_lvgl` valida o padrão flush.

**Alternatives considered**:
- LVGL “bare metal” sem port — mais controle, mais código boilerplate; rejeitado.
- Adafruit_GFX — inadequado para layout rico e alternância de cenas.

**Contract**:
- `LV_COLOR_DEPTH=16`
- Heap LVGL 48 KB, alocação PSRAM quando `CONFIG_SPIRAM` ativo
- `min_refresh_rate` hub75: 60 Hz
- Sem `double_buffer` no hub75

---

## 3. Sincronização de tempo (SNTP)

**Decision**: `esp_netif_sntp` (ESP-IDF v6.x API) com servidores `pool.ntp.org` + `time.google.com`; fuso via string IANA em NVS (`tz`) aplicada com `setenv("TZ", ...)` + `tzset()` após sync.

**Rationale**:
- API estável ESP-IDF; suporta callback de sync.
- Fuso configurável atende FR-004; default `America/Sao_Paulo`.

**Alternatives considered**:
- Armazenar offset fixo UTC−3 — não lida com horário de verão; rejeitado.
- GPS/time local sem rede — fora de escopo.

**Behavior**:
- Estado `CLOCK_SYNC_PENDING` até primeiro sync; tela mostra “Sem hora”.
- Re-sync periódico SNTP padrão IDF (24 h).

---

## 4. Open-Meteo — forecast e geocoding

**Decision**:
- **Clima atual**: `GET https://api.open-meteo.com/v1/forecast?latitude={lat}&longitude={lon}&current=temperature_2m,weather_code&timezone={tz}`
- **Geocoding** (quando NVS tem cidade mas não lat/lon): `GET https://geocoding-api.open-meteo.com/v1/search?name={city}&count=1&language=pt`
- Parser JSON mínimo em `protocol/` (cJSON do ESP-IDF).

**Rationale**:
- API gratuita, sem chave; HTTPS; campos mínimos para 128×64.
- Persistir lat/lon após geocode evita chamada repetida a cada boot.

**Alternatives considered**:
- OpenWeatherMap — exige API key; rejeitado para marco inicial.
- Armazenar só lat/lon sem nome de cidade — pior UX de diagnóstico; manter ambos.

**Refresh**: timer 30 min (`WEATHER_FETCH_INTERVAL_S=1800`); timeout HTTP 10 s; 3 retries com backoff 5/15/45 s.

---

## 5. Condição climática em português

**Decision**: Tabela estática WMO weather code → string PT curta (≤ 12 chars) em `protocol/wmo_condition_pt.c`; ex.: `0→"Limpo"`, `61→"Chuva"`, `95→"Tempest"`.

**Rationale**:
- Open-Meteo retorna `weather_code` numérico WMO; tradução local evita dependência de campo textual da API.
- Testável sem rede.

**Alternatives considered**:
- `language=pt` no geocoding apenas — não traduz weather_code; insuficiente.

---

## 6. NVS schema e cache

**Decision**: Namespace `painel_cfg` versão 1 — ver [contracts/nvs-schema.md](./contracts/nvs-schema.md).

**Rationale**:
- Separa config de produto de credenciais Wi-Fi.
- Cache de clima permite SC-004 (offline 30 min).
- Chave `cfg_ver` permite migração futura.

**Alternatives considered**:
- SPIFFS/LittleFS para cache — overhead desnecessário para ~32 bytes; rejeitado.

---

## 7. Wi-Fi (marco inicial)

**Decision**: STA mode; credenciais NVS `wifi` + fallback Kconfig para dev; reconexão com backoff exponencial (1→60 s cap); eventos publicados para `app_task` via fila.

**Rationale**:
- Mínimo para SNTP e clima; sem BLE neste marco.
- Kconfig acelera bring-up em bancada sem gravador NVS custom.

**Alternatives considered**:
- Blufi — fora de escopo spec FR-013.
- Hardcode SSID no código — rejeitado (segredo no repo).

---

## 8. Alternância de cenas

**Decision**: `esp_timer` periódico 10 s em `app_controller`; troca visibilidade/objetos LVGL das scenes clock/weather (duas telas ou dois containers na mesma screen).

**Rationale**:
- Simples, previsível, atende SC-006.
- Independente do loop LVGL de 1 s do relógio.

**Alternatives considered**:
- Gestos/botões MatrixPortal — UX não solicitada; adiar.

---

## 9. Partições e memória

**Decision**: Tabela de partições **factory default** ESP-IDF (8 MB flash); sem OTA custom neste marco.

**Rationale**:
- Constituição deferiu OTA; firmware único cabe com LVGL+Wi-Fi.
- Validar com `idf.py size` após implementação.

**sdkconfig highlights** (a aplicar em `sdkconfig.defaults.esp32s3`):
- `CONFIG_SPIRAM=y` (MatrixPortal S3 tem PSRAM)
- `CONFIG_LV_MEM_CUSTOM`, alocador PSRAM
- Wi-Fi STA, SNTP, HTTP client enabled

---

## 10. Testes sem hardware

**Decision**: Unity tests em host ou `idf.py test` para:
- Parse JSON forecast/geocode (fixtures em flash ou strings embutidas)
- Mapeamento WMO→PT

**Rationale**:
- Atende constituição IV (testabilidade protocol/domain).

**Hardware-only** (quickstart): legibilidade 1 m, scan driver, layout físico 128×64 vs 2×2 64×32.

---

## 11. Painel totalmente preto — diagnóstico em camadas

**Decision**: Executar programa de bring-up em **5 tiers** (serial → elétrico → HUB75 fill → LVGL smoke → app UI) com matriz sdkconfig M0–M7; firmware opcional com `CONFIG_PAINEL_DISPLAY_BRINGUP`.

**Rationale**:
- Campo reporta serial saudável (Wi-Fi, SNTP, clima) mas **zero pixels visíveis** — falha pode estar em DMA/scan (hardware config), flush LVGL, ou UI oculta; tiers isolam cada camada sem alterar GPIO.
- `Hub75Driver::fill()` e `clear()` permitem teste bare-metal antes de LVGL — padrão recomendado em bring-up HUB75 upstream.
- Matriz M2–M5 cobre causas frequentes: shift driver FM6126A, wiring 1/8 scan, grade 2×2 64×32 (já prevista na spec de fallback).

**Alternatives considered**:
- Debug only via LVGL (pular HUB75 bare-metal) — não distingue scan errado de flush errado; rejeitado.
- Alterar pinagem manualmente — viola constituição; rejeitado.
- `CONFIG_HUB75_DEBUG_TIMING` apenas — útil secundário, insuficiente sozinho; complementar a Tier 2.

**Root-cause → fix mapping**:

| Sintoma observado | Provável causa | Ação |
|-------------------|----------------|------|
| T2 red FAIL, logs DMA OK | Power/cable, shift driver, scan layout | Tier 1 + M2/M4/M5 |
| T2 PASS, T3 FAIL | Flush LVGL (stride, mutex, `flush_ready`) | Revisar `display_lvgl.c` |
| T3 PASS, T4 FAIL | Scenes/splash timing, labels hidden | Revisar `app_controller`, `scene_*` |
| Cores permutadas | RGB order / wiring | M4, documentar panel SKU |
| Faixas horizontais | Scan rate / wiring 1/8 | M4 |
| Imagem fragmentada 4 quadrantes | Módulo 64×32 em 2×2 | M5 |
| Tudo OK no serial, preto total | H3 framebuffer zero até primeiro flush; forçar fill no boot | Tier 2.1 imediato |

**Contract**: [contracts/display-bringup-debug.md](./contracts/display-bringup-debug.md)
