#include "open_meteo.h"

#include "connectivity.h"
#include "esp_log.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "open_meteo";

static const struct {
    int code;
    const char *pt;
} s_wmo[] = {
    {0, "Limpo"},
    {1, "Quase limpo"},
    {2, "Parcial nub"},
    {3, "Nublado"},
    {45, "Neblina"},
    {48, "Neblina"},
    {51, "Garoa"},
    {61, "Chuva"},
    {63, "Chuva"},
    {65, "Chuva forte"},
    {71, "Neve"},
    {80, "Pancadas"},
    {95, "Tempestade"},
};

const char *wmo_condition_pt(int code)
{
    for (size_t i = 0; i < sizeof(s_wmo) / sizeof(s_wmo[0]); i++) {
        if (s_wmo[i].code == code) {
            return s_wmo[i].pt;
        }
    }
    return "Clima";
}

static const char *find_key_in_object(const char *json, const char *object_key, const char *field_key)
{
    char obj_pattern[32];
    snprintf(obj_pattern, sizeof(obj_pattern), "\"%s\":", object_key);
    const char *obj = strstr(json, obj_pattern);
    if (obj == NULL) {
        return NULL;
    }
    obj = strchr(obj, '{');
    if (obj == NULL) {
        return NULL;
    }

    char field_pattern[48];
    snprintf(field_pattern, sizeof(field_pattern), "\"%s\":", field_key);
    return strstr(obj, field_pattern);
}

esp_err_t open_meteo_fetch_current(float lat, float lon, const char *tz, open_meteo_current_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(out, 0, sizeof(*out));

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,"
             "weather_code&timezone=%s",
             lat, lon, tz ? tz : "America/Sao_Paulo");

    char *body = malloc(4096);
    if (body == NULL) {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = http_get_string(url, body, 4096);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "http fail");
        free(body);
        return err;
    }

    const char *temp_ptr = find_key_in_object(body, "current", "temperature_2m");
    const char *code_ptr = find_key_in_object(body, "current", "weather_code");
    if (temp_ptr == NULL || code_ptr == NULL) {
        free(body);
        return ESP_FAIL;
    }
    temp_ptr = strchr(temp_ptr, ':');
    code_ptr = strchr(code_ptr, ':');
    if (temp_ptr == NULL || code_ptr == NULL) {
        free(body);
        return ESP_FAIL;
    }
    out->temp_c = (int)lround(strtod(temp_ptr + 1, NULL));
    out->weather_code = (int)strtol(code_ptr + 1, NULL, 10);
    out->valid = true;
    free(body);
    ESP_LOGI(TAG, "weather %dC code=%d", out->temp_c, out->weather_code);
    return ESP_OK;
}
