#include "painel_storage.h"
#include "painel_nvs_schema.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <string.h>
#include <time.h>

static const char *TAG = "storage";

esp_err_t painel_storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t painel_storage_load_config(painel_config_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    strncpy(out->tz, PAINEL_DEFAULT_TZ, sizeof(out->tz) - 1);
    out->dwell_ms = PAINEL_DEFAULT_DWELL_MS;

    nvs_handle_t h;
    esp_err_t err = nvs_open(PAINEL_NVS_NS_CFG, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return ESP_OK;
    }

    size_t len = sizeof(out->tz);
    nvs_get_str(h, PAINEL_KEY_TZ, out->tz, &len);
    nvs_get_u32(h, PAINEL_KEY_DWELL_MS, &out->dwell_ms);
    nvs_close(h);
    return ESP_OK;
}

esp_err_t painel_storage_save_config(const painel_config_t *cfg)
{
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(PAINEL_NVS_NS_CFG, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    uint8_t ver = PAINEL_CFG_VERSION;
    nvs_set_u8(h, PAINEL_KEY_CFG_VER, ver);
    nvs_set_str(h, PAINEL_KEY_TZ, cfg->tz);
    nvs_set_u32(h, PAINEL_KEY_DWELL_MS, cfg->dwell_ms);
    err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t painel_storage_load_wifi(painel_wifi_creds_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));

    nvs_handle_t h;
    esp_err_t err = nvs_open(PAINEL_NVS_NS_WIFI, NVS_READONLY, &h);
    if (err != ESP_OK) {
        if (strlen(CONFIG_PAINEL_WIFI_SSID) > 0) {
            strncpy(out->ssid, CONFIG_PAINEL_WIFI_SSID, sizeof(out->ssid) - 1);
            strncpy(out->pass, CONFIG_PAINEL_WIFI_PASSWORD, sizeof(out->pass) - 1);
            return ESP_OK;
        }
        return ESP_ERR_NOT_FOUND;
    }

    size_t len = sizeof(out->ssid);
    err = nvs_get_str(h, PAINEL_KEY_WIFI_SSID, out->ssid, &len);
    if (err != ESP_OK || out->ssid[0] == '\0') {
        nvs_close(h);
        if (strlen(CONFIG_PAINEL_WIFI_SSID) > 0) {
            strncpy(out->ssid, CONFIG_PAINEL_WIFI_SSID, sizeof(out->ssid) - 1);
            strncpy(out->pass, CONFIG_PAINEL_WIFI_PASSWORD, sizeof(out->pass) - 1);
            return ESP_OK;
        }
        return ESP_ERR_NOT_FOUND;
    }
    len = sizeof(out->pass);
    nvs_get_str(h, PAINEL_KEY_WIFI_PASS, out->pass, &len);
    nvs_close(h);
    ESP_LOGI(TAG, "wifi creds loaded (ssid masked)");
    return ESP_OK;
}

esp_err_t painel_storage_load_weather_cache(painel_weather_cache_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));

    nvs_handle_t h;
    esp_err_t err = nvs_open(PAINEL_NVS_NS_CFG, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }
    int16_t t = 0;
    int16_t code = 0;
    int64_t ts = 0;
    if (nvs_get_i16(h, PAINEL_KEY_WX_T_X10, &t) == ESP_OK && nvs_get_i16(h, PAINEL_KEY_WX_CODE, &code) == ESP_OK &&
        nvs_get_i64(h, PAINEL_KEY_WX_TS, &ts) == ESP_OK) {
        out->temp_x10 = t;
        out->weather_code = code;
        out->fetched_unix = ts;
        out->valid = true;
    }
    nvs_close(h);
    return ESP_OK;
}

esp_err_t painel_storage_save_weather_cache(const painel_weather_cache_t *cache)
{
    if (cache == NULL || !cache->valid) {
        return ESP_ERR_INVALID_ARG;
    }
    nvs_handle_t h;
    esp_err_t err = nvs_open(PAINEL_NVS_NS_CFG, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    nvs_set_i16(h, PAINEL_KEY_WX_T_X10, cache->temp_x10);
    nvs_set_i16(h, PAINEL_KEY_WX_CODE, cache->weather_code);
    nvs_set_i64(h, PAINEL_KEY_WX_TS, cache->fetched_unix);
    err = nvs_commit(h);
    nvs_close(h);
    return err;
}

void painel_tz_apply(const char *tz)
{
    const char *posix = PAINEL_POSIX_TZ;
    if (tz != NULL && tz[0] != '\0' && strchr(tz, '/') == NULL) {
        posix = tz;
    }
    setenv("TZ", posix, 1);
    tzset();
    ESP_LOGI(TAG, "timezone %s (from %s)", posix, tz && tz[0] ? tz : PAINEL_DEFAULT_TZ);
}
