#pragma once

#include "hub75.h"
#include "esp_log.h"
#include "sdkconfig.h"

static inline Hub75Config hub75_get_matrixportal_config(void)
{
    Hub75Config config = {};
    config.panel_width = CONFIG_HUB75_PANEL_WIDTH;
    config.panel_height = CONFIG_HUB75_PANEL_HEIGHT;
    config.scan_wiring = Hub75ScanWiring::STANDARD_TWO_SCAN;
    config.shift_driver = Hub75ShiftDriver::FM6126A;
    config.layout_rows = CONFIG_HUB75_LAYOUT_ROWS;
    config.layout_cols = CONFIG_HUB75_LAYOUT_COLS;
    config.layout = Hub75PanelLayout::HORIZONTAL;
    config.min_refresh_rate = CONFIG_HUB75_MIN_REFRESH_RATE;
    config.brightness = CONFIG_HUB75_BRIGHTNESS;
    config.double_buffer = false;
    config.latch_blanking = CONFIG_HUB75_LATCH_BLANKING;

#if CONFIG_HUB75_CLK_10MHZ
    config.output_clock_speed = Hub75ClockSpeed::HZ_10M;
#elif CONFIG_HUB75_CLK_16MHZ
    config.output_clock_speed = Hub75ClockSpeed::HZ_16M;
#else
    config.output_clock_speed = Hub75ClockSpeed::HZ_20M;
#endif

    config.pins.r1 = CONFIG_HUB75_PIN_R1;
    config.pins.g1 = CONFIG_HUB75_PIN_G1;
    config.pins.b1 = CONFIG_HUB75_PIN_B1;
    config.pins.r2 = CONFIG_HUB75_PIN_R2;
    config.pins.g2 = CONFIG_HUB75_PIN_G2;
    config.pins.b2 = CONFIG_HUB75_PIN_B2;
    config.pins.a = CONFIG_HUB75_PIN_A;
    config.pins.b = CONFIG_HUB75_PIN_B;
    config.pins.c = CONFIG_HUB75_PIN_C;
    config.pins.d = CONFIG_HUB75_PIN_D;
    config.pins.e = CONFIG_HUB75_PIN_E;
    config.pins.lat = CONFIG_HUB75_PIN_LAT;
    config.pins.oe = CONFIG_HUB75_PIN_OE;
    config.pins.clk = CONFIG_HUB75_PIN_CLK;

    ESP_LOGI("hub75_board", "matrixportal %ux%u FM6126A", (unsigned)config.panel_width,
             (unsigned)config.panel_height);
    return config;
}
