/**
 * @file diagnostics.h
 * @brief Non-writing basic, seek and endurance diagnostics with cleanup.
 */
#pragma once
#include "esp_err.h"
/**
 * @brief Run non-writing sensor, motor, RPM and formatted-disk activity checks.
 * @return ESP_OK if all checks pass; controls are released on valid-request completion.
 */
esp_err_t diagnostics_basic(void);
/**
 * @brief Visit representative legal tracks and verify a tracked return to zero.
 * @return ESP_OK if all selected targets succeed; all completion paths release controls.
 */
esp_err_t diagnostics_seek(void);
/**
 * @brief Repeat zero-to-maximum-to-zero motion with cancellation and cycle accounting.
 *
 * @param cycles Complete motion cycles requested, 1–100000.
 * @return ESP_OK for all cycles or the first error; valid requests always release controls.
 */
esp_err_t diagnostics_exercise(unsigned cycles);
