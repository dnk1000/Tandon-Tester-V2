/**
 * @file config.h
 * @brief Validated RAM settings and versioned NVS persistence.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/**
 * @brief Complete persisted configuration, validated as one record.
 * @details step_time, motor_spinup_ms and read_sample_ms use milliseconds.
 * max_track is inclusive; home_limit counts pulses; rpm_samples counts intervals.
 * drive_select is 0–3; rpm_tolerance is percentage (1.5 means 1.5 percent).
 */
typedef struct {
    uint32_t step_time, max_track, home_limit, rpm_samples, drive_select;
    uint32_t motor_spinup_ms, read_sample_ms;
    float rpm_tolerance;
} tester_config_t;
/**
 * @brief Populate conservative TM-100-1 Revision A settings.
 *
 * @param c Non-NULL configuration candidate or destination.
 */
void config_defaults(tester_config_t *c);
/**
 * @brief Validate every field and cross-field limits before accepting settings.
 *
 * @param c Candidate settings; NULL is rejected.
 * @return true only when every setting and cross-field relationship is valid.
 */
bool config_validate(const tester_config_t *c);
/**
 * @brief Load defaults first and replace them only with a valid versioned NVS record.
 * @return ESP_OK for loaded settings or default fallback; initialization/open errors otherwise.
 */
esp_err_t config_init(void);
/**
 * @brief Borrow the active RAM configuration without transferring ownership.
 * @return Borrowed read-only settings; the command worker owns access after startup.
 */
const tester_config_t *config_get(void);
/**
 * @brief Validate and replace RAM settings without persisting them.
 *
 * @param c Non-NULL configuration candidate or destination.
 * @return ESP_OK if RAM settings changed, or ESP_ERR_INVALID_ARG. Caller handles motion cleanup.
 */
esp_err_t config_apply(const tester_config_t *c);
/**
 * @brief Validate and commit a complete versioned configuration record to NVS.
 * @return ESP_OK after commit, or validation, initialization, open, write or commit error.
 */
esp_err_t config_save(void);
