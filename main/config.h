#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct {
    uint32_t step_time, max_track, home_limit, rpm_samples, drive_select;
    uint32_t motor_spinup_ms, read_sample_ms;
    float rpm_tolerance;
} tester_config_t;
void config_defaults(tester_config_t *c);
bool config_validate(const tester_config_t *c);
esp_err_t config_init(void);
const tester_config_t *config_get(void);
esp_err_t config_apply(const tester_config_t *c);
esp_err_t config_save(void);
