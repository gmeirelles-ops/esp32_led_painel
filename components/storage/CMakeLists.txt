idf_component_register(
    SRCS "painel_storage.c"
    INCLUDE_DIRS "include" "."
    REQUIRES nvs_flash
)
