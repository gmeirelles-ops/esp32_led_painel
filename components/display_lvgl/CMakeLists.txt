idf_component_register(
    SRCS "display_lvgl.c"
    INCLUDE_DIRS "include"
    REQUIRES hub75_driver lvgl esp_timer freertos esp_system
)
