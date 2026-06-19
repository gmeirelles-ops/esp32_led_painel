idf_component_register(
    SRCS "app_controller.c" "dashboard_state.c" "scene_clock.c" "scene_weather.c"
    INCLUDE_DIRS "include"
    REQUIRES display_lvgl connectivity protocol storage lvgl esp_timer
)
