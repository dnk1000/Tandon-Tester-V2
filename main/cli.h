/**
 * @file cli.h
 * @brief Native USB command input, execution and cancellation.
 */
#pragma once
#include "esp_err.h"
/**
 * @brief Install native USB console I/O and start the input and command-worker tasks.
 * @return ESP_OK when tasks start, or a driver/allocation error; call once after HAL setup.
 */
esp_err_t cli_start(void);
