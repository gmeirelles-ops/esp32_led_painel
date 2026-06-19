#pragma once

#include "hub75.h"
#include "esp_log.h"
#include "sdkconfig.h"

static inline Hub75Config hub75_get_matrixportal_config(void)
{
    Hub75Config config = {};
    config.panel_width = 128;
    config.panel_height = 64;
    config.scan_wiring = Hub75ScanWiring::STANDARD_TWO_SCAN;
    config.shift_driver = Hub75ShiftDriver::FM6126A;

#if CONFIG_PAINEL_TOPOLOGY_2X2
    config.layout_rows = 2;
    config.layout_cols = 2;
    config.layout = Hub75PanelLayout::TOP_LEFT_DOWN_ZIGZAG;
    const char *topology = "2x2";
#else
    config.layout_rows = 1;
    config.layout_cols = 1;
    config.layout = Hub75PanelLayout::HORIZONTAL;
    const char *topology = "single";
#endif

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

    const unsigned virtual_w = config.panel_width * config.layout_cols;
    const unsigned virtual_h = config.panel_height * config.layout_rows;
    ESP_LOGI("hub75_board", "topology %s panel %ux%u layout %ux%u virtual %ux%u", topology,
             (unsigned)config.panel_width, (unsigned)config.panel_height, (unsigned)config.layout_rows,
             (unsigned)config.layout_cols, virtual_w, virtual_h);
    return config;
}
