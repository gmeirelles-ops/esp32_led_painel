#include "connectivity.h"

#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "painel_storage.h"
#include <string.h>
#include <time.h>

static const char *TAG_WIFI = "wifi_mgr";
static const char *TAG_SNTP = "sntp_svc";
static const char *TAG_HTTP = "http_cli";

static EventGroupHandle_t s_wifi_events;
static bool s_wifi_connected;
static bool s_sntp_synced;
static app_connectivity_cb_t s_cb;
static void *s_cb_ctx;

#define WIFI_CONNECTED_BIT BIT0

void wifi_manager_set_callback(app_connectivity_cb_t cb, void *ctx)
{
    s_cb = cb;
    s_cb_ctx = ctx;
}

static void notify(app_connectivity_event_t evt)
{
    if (s_cb) {
        s_cb(evt, s_cb_ctx);
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_connected = false;
        xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG_WIFI, "disconnected, reconnecting");
        notify(APP_EVT_WIFI_DOWN);
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_wifi_connected = true;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG_WIFI, "connected");
        notify(APP_EVT_WIFI_UP);
    }
}

esp_err_t wifi_manager_init(void)
{
    s_wifi_events = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));
    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    painel_wifi_creds_t creds;
    esp_err_t err = painel_storage_load_wifi(&creds);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "no wifi creds");
        return err;
    }

    wifi_config_t wcfg = {0};
    strncpy((char *)wcfg.sta.ssid, creds.ssid, sizeof(wcfg.sta.ssid) - 1);
    strncpy((char *)wcfg.sta.password, creds.pass, sizeof(wcfg.sta.password) - 1);
    wcfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wcfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    return ESP_OK;
}

bool wifi_manager_is_connected(void)
{
    return s_wifi_connected;
}

static void sntp_sync_cb(struct timeval *tv)
{
    (void)tv;
    s_sntp_synced = true;
    ESP_LOGI(TAG_SNTP, "time synced");
    notify(APP_EVT_SNTP_SYNCED);
}

esp_err_t sntp_service_start(const char *tz)
{
    if (tz && tz[0]) {
        setenv("TZ", tz, 1);
        tzset();
    }
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(sntp_sync_cb);
    esp_sntp_init();
    return ESP_OK;
}

bool sntp_service_is_synced(void)
{
    if (!s_sntp_synced) {
        time_t now = 0;
        struct tm tm_info = {0};
        time(&now);
        localtime_r(&now, &tm_info);
        s_sntp_synced = (tm_info.tm_year + 1900) > 2020;
    }
    return s_sntp_synced;
}

esp_err_t http_get_string(const char *url, char *out, size_t out_len)
{
    if (url == NULL || out == NULL || out_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    out[0] = '\0';

    esp_http_client_config_t cfg = {
        .url = url,
        .timeout_ms = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (client == NULL) {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        return err;
    }
    int len = esp_http_client_fetch_headers(client);
    if (len < 0 || (size_t)len >= out_len) {
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }
    int read = esp_http_client_read(client, out, out_len - 1);
    esp_http_client_cleanup(client);
    if (read <= 0) {
        return ESP_FAIL;
    }
    out[read] = '\0';
    return ESP_OK;
}
