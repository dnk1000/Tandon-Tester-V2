/**
 * @file nvs.h
 * @brief Host-only nvs fixture or declaration; not a physical hardware validation.
 */
#pragma once
#include <stddef.h>
#include "esp_err.h"
typedef unsigned nvs_handle_t;
#define NVS_READONLY 0
#define NVS_READWRITE 1
#define ESP_ERR_NVS_NOT_FOUND 10
/**
 * @brief Validate the namespace and model opening the fixture record.
 */
esp_err_t nvs_open(const char *, int, nvs_handle_t *);
/**
 * @brief Copy the fixture record with absence and buffer-size checks.
 */
esp_err_t nvs_get_blob(nvs_handle_t, const char *, void *, size_t *);
/**
 * @brief Store a fixture record or return an injected write error.
 */
esp_err_t nvs_set_blob(nvs_handle_t, const char *, const void *, size_t);
/**
 * @brief Count commit attempts and return an injected result.
 */
esp_err_t nvs_commit(nvs_handle_t);
/**
 * @brief Verify that the expected fixture handle is closed.
 */
void nvs_close(nvs_handle_t);
