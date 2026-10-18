/**
 * @file esp_err.h
 * @brief Host-only esp err fixture or declaration; not a physical hardware validation.
 */
#pragma once
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_INVALID_STATE 2
#define ESP_ERR_INVALID_RESPONSE 3
#define ESP_ERR_TIMEOUT 4
#define ESP_ERR_NOT_FOUND 5
#define ESP_ERR_NO_MEM 6
