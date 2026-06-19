idf_component_register(
    SRCS "esp32_led_painel.c"
    INCLUDE_DIRS "."
    REQUIRES hub75_driver display_lvgl storage connectivity app
)
