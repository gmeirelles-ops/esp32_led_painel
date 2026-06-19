#pragma once

#include <stdint.h>

typedef enum {
    DASH_SCENE_INIT = 0,
    DASH_SCENE_CLOCK,
    DASH_SCENE_WEATHER,
} dash_scene_t;

typedef struct {
    dash_scene_t active;
    dash_scene_t previous;
    uint32_t dwell_ms;
    bool init_complete;
    bool rotation_enabled;
} dashboard_state_t;

void dashboard_state_init(dashboard_state_t *st, uint32_t dwell_ms);
void dashboard_state_set_active(dashboard_state_t *st, dash_scene_t scene);
dash_scene_t dashboard_state_toggle(dashboard_state_t *st);
