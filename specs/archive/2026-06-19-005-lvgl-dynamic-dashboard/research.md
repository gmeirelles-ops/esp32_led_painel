# Research: Dashboard LVGL Dinâmico

## Regressão — hora não aparece

**Decision**: Implementar `connectivity_bootstrap()` idempotente no início de `app_controller_start`.

**Root cause**: Em `main/esp32_led_painel.c`, `wifi_manager_start()` executa antes de `app_controller_start()`. Se o STA obtém IP rapidamente, `IP_EVENT_STA_GOT_IP` dispara antes de `wifi_manager_set_callback()` registrar `on_connectivity`. O evento `APP_EVT_WIFI_UP` é perdido → `sntp_service_start()` nunca é chamado → `scene_clock` permanece em `CLOCK_SYNC_PENDING` ou exibe "Sem hora" indefinidamente.

**Rationale**: Padrão comum em firmware ESP-IDF; bootstrap explícito do estado atual após registrar handlers é mais confiável que reordenar init global.

**Alternatives considered**:
- Mover callback antes de `wifi_manager_start` em `main`: exigiria passar config/tz para main — viola separação de camadas.
- Polling SNTP em `scene_clock`: mascara o bug de conectividade; rejeitado.

**Secondary fixes**:
- `painel_tz_apply` + POSIX `BRT3` (já implementado em sessão anterior) para UTC vs local.
- Pós-sync: log hora local e `scene_clock_set_sync(SYNCED)` com refresh imediato.
- Validar `tm_year > 2020` antes de mostrar hora; senão "Sincronizando…".

## Layout responsivo 128×64 vs 256×128

**Decision**: Módulo `scene_layout` com breakpoint em `display_lvgl_hor_res() > 128` → perfil `large`.

**Rationale**: Fontes bitmap escalam melhor que `transform_scale` em painel LED (evita blur/subpixel). Montserrat 48 legível em 256×128; 24 mantém 128×64 validado.

**Alternatives considered**:
- `lv_obj_set_style_transform_scale(512)` (2×): artefatos em matriz LED; rejeitado.
- Uma única fonte 24 em ambos: falha SC-002 em 2×2; rejeitado.

**Font config**: Adicionar `CONFIG_LV_FONT_MONTSERRAT_48=y` em `sdkconfig.defaults.esp32s3`; 12/24 já habilitados.

## Transições entre cenas

**Decision**: Fade out/in 250 ms + 250 ms (total 500 ms) via `lv_anim_t` em opacidade dos roots `scene_clock` / `scene_weather`.

**Rationale**: Leve em CPU; sem alocação extra; atende FR-005/SC-003.

**Alternatives considered**:
- `lv_screen_load_anim` com telas LVGL separadas: refactor maior; deferido.
- Slide horizontal: legibilidade ruim em 128×64 durante movimento; rejeitado.

**Edge case**: Se animação em curso e dwell dispara, cancelar anim anterior (`lv_anim_delete`) antes de nova troca.

## Estados visuais

**Decision**:

| Estado | Clock | Weather |
|--------|-------|---------|
| Pending | "Sincronizando…" | — |
| Synced | HH:MM + data | temp + condição |
| No Wi-Fi | "Sem Wi-Fi" | "Sem Wi-Fi" |
| Error | "Sem hora" | "Sem clima" |
| Stale | — | prefixo `~`, opa 70% no label temp |

**Rationale**: Atende FR-006/SC-004 sem novos assets gráficos.

## Memória / heap

**Decision**: Sem alocação dinâmica adicional; animações usam stack LVGL existente.

**Validation**: `idf.py size` após habilitar Montserrat 48; esperado +~20–40 KB flash, heap estável (framebuffer já em PSRAM via 004).
