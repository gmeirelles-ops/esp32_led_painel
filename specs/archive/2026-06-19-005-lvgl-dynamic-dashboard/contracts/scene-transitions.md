# Contract: Scene Transitions

**Version**: 1.0 | **Feature**: `005-lvgl-dynamic-dashboard`

## Behavior

On automatic dwell toggle (10 s default):

1. Fade out active scene root (250 ms)
2. Hide outgoing, show incoming
3. Fade in incoming scene root (250 ms)

Total ≤ 500 ms.

## Implementation constraints

- Run animations under `display_lvgl_lock()` or via `lv_async_call`
- Cancel in-flight animation before starting new transition
- Rotation disabled during splash (`init_complete == false`)

## Logging

```
I app_ctrl: scene transition clock -> weather
I app_ctrl: scene transition weather -> clock
```

## Failure

If animation fails to start, fall back to instant `apply_scene()` (no blank >500 ms).
