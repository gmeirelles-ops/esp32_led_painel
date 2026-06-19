#include "dashboard_state.h"

void dashboard_state_init(dashboard_state_t *st, uint32_t dwell_ms)
{
    if (st == NULL) {
        return;
    }
    st->active = DASH_SCENE_INIT;
    st->previous = DASH_SCENE_INIT;
    st->dwell_ms = dwell_ms > 0 ? dwell_ms : 10000;
    st->init_complete = false;
    st->rotation_enabled = false;
}

void dashboard_state_set_active(dashboard_state_t *st, dash_scene_t scene)
{
    if (st == NULL) {
        return;
    }
    st->previous = st->active;
    st->active = scene;
}

dash_scene_t dashboard_state_toggle(dashboard_state_t *st)
{
    dash_scene_t next = (st->active == DASH_SCENE_CLOCK) ? DASH_SCENE_WEATHER : DASH_SCENE_CLOCK;
    dashboard_state_set_active(st, next);
    return next;
}
