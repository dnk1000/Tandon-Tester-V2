/**
 * @file nvs_flash.h
 * @brief Host-only nvs flash fixture or declaration; not a physical hardware validation.
 */
#pragma once
#include "esp_err.h"
/**
 * @brief Return an injected initialization result without touching flash.
 */
esp_err_t nvs_flash_init(void);
