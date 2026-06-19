# esp32_led_painel

Firmware em C/C++ para painel LED **HUB75 128×64** no **Adafruit MatrixPortal S3** (ESP32-S3), usando ESP-IDF e GitHub Spec Kit.

- **Target**: `esp32s3`
- **Board**: MatrixPortal S3 + chicote HUB75 (preset da placa)
- **Painel**: 128×64, scan 1/32, HUB75E (ex.: SXHL-P2.5-1735)
- **Driver**: `esphome/esp-hub75`, shift **FM6126A**
- **Milestone 0**: exibir **DIPONTO** (texto bitmap, sem LVGL)
- **Constituição**: v2.0.0
- **Feature ativa**: `specs/002-diponto-text/`
- **Build**: `idf.py -B ~/esp32_led_painel-build build`

Feature `001-lvgl-clock-weather` está **superseded** (histórico apenas).

---