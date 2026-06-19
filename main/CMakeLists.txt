idf_component_register(
    SRCS "esp32_led_painel.c" "diponto_font.c"
    INCLUDE_DIRS "."
    REQUIRES hub75_driver
)
