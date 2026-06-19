# Research: HUB75 2×2 Topology

## Layout type for 2×2 grid

**Decision**: `Hub75PanelLayout::TOP_LEFT_DOWN_ZIGZAG` for 2×2 profile.

**Rationale**: All four panels mounted upright; row 0 chains left→right, row 1 chains right→left (zigzag). Matches common HUB75 wall wiring when alternate rows are not flipped. `esp-hub75` includes unit tests for 2×2 zigzag with 128×64 panels mapping corners correctly.

**Alternatives considered**:
- `TOP_LEFT_DOWN` (serpentine): alternate rows upside down — shorter cables but requires physical panel rotation; deferred to v2 Kconfig if hardware validation fails.
- `HORIZONTAL`: only valid for `layout_rows=1`; not applicable.

## Memory (DMA + LVGL)

**Decision**: LVGL full-frame buffer allocated via `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`; size = `width × height × 2` bytes.

**Rationale**:
- Single: 128×64 = 16 KB RGB565
- 2×2: 256×128 = 64 KB RGB565
- MatrixPortal S3 has quad PSRAM; `CONFIG_SPIRAM_USE_MALLOC=y` already enabled.
- HUB75 DMA buffers are allocated internally by `esp-hub75` (4× pixel count for 2×2); PSRAM is the appropriate pool.

**Alternatives considered**:
- Static max-size buffer (64 KB always): wastes 48 KB in single mode — rejected.
- Partial LVGL buffer: would complicate flush; full-frame mode already in use — keep pattern.

## Refresh rate

**Decision**: Keep `CONFIG_HUB75_MIN_REFRESH_RATE=60` for both profiles initially.

**Rationale**: Driver auto-adjusts bit depth for target refresh. 4× pixels increases DMA work; if flicker observed on hardware, reduce to 30 Hz in sdkconfig profile fragment — documented in quickstart as tuning step.

**Alternatives considered**:
- Default 30 Hz for 2×2 only: premature without hardware evidence — defer to validation.

## Physical chaining (2×2)

**Decision**: Row-major chain, zigzag layout as above.

Panel index order (virtual coordinates):

```text
[0: top-left]  [1: top-right]
[2: bot-left]  [3: bot-right]
```

Data flows: panel 0 → 1 (top row), then → panel 2 (bottom-right of chain per zigzag), → panel 3.

Documented in [quickstart.md](./quickstart.md) for field wiring verification.
