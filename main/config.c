#include "config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
static tester_config_t current;
static bool nvs_ready;
#define CONFIG_VERSION 1u
typedef struct { uint32_t version; tester_config_t config; } saved_config_t;
esp_err_t config_init(void)
{
    nvs_ready=false;
    config_defaults(&current);
    esp_err_t err = nvs_flash_init();
    // Preserve other namespaces on full/incompatible NVS; never erase automatically.
    if (err != ESP_OK) return err;
    nvs_ready = true;
    nvs_handle_t h;
    err = nvs_open("tm100", NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;
    saved_config_t saved = {0};
    size_t size = sizeof(saved);
    err = nvs_get_blob(h, "settings", &saved, &size);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err == ESP_OK && size == sizeof(saved) && saved.version == CONFIG_VERSION &&
        config_validate(&saved.config)) {
        current = saved.config;
    } else {
        ESP_LOGW("config", "Invalid/unreadable saved settings; using defaults");
    }
    return ESP_OK;
}
const tester_config_t *config_get(void) { return &current; }
esp_err_t config_apply(const tester_config_t *c)
{
    if (!config_validate(c)) return ESP_ERR_INVALID_ARG;
    current = *c;
    return ESP_OK;
}
esp_err_t config_save(void)
{
    if (!nvs_ready) return ESP_ERR_INVALID_STATE;
    if (!config_validate(&current)) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = nvs_open("tm100", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    saved_config_t saved = {.version=CONFIG_VERSION, .config=current};
    err = nvs_set_blob(h, "settings", &saved, sizeof(saved));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}
