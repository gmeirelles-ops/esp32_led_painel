idf_component_register(
    SRCS "hub75_driver.cpp"
    INCLUDE_DIRS "include" "."
    REQUIRES esphome__esp-hub75
)

idf_component_get_property(_hub75_lib esphome__esp-hub75 COMPONENT_LIB)
target_compile_options(${_hub75_lib} PRIVATE -Wno-error=missing-field-initializers)
