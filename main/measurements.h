#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct {
    bool active, valid;
    int64_t last_rise, period_us, width_us, min_us, max_us;
    uint32_t pulses;
} index_reading_t;
typedef struct {
    unsigned samples;
    double rpm, rpm_min, rpm_max, deviation, period_min_us, period_max_us, jitter_us;
    bool passed;
} rpm_reading_t;
typedef struct { uint32_t pulses; int64_t duration_us; double rate; } activity_reading_t;
typedef struct {
    uint32_t samples, min_ticks, max_ticks, histogram[16];
    uint64_t sum_ticks;
} timing_reading_t;
// Portable streaming rising-edge interval reducer, also exercised by host tests.
typedef struct {
    timing_reading_t result;
    uint64_t elapsed, last_rise;
    int previous_level;
    bool have_rise;
} timing_accumulator_t;
void timing_accumulator_init(timing_accumulator_t *a);
void timing_accumulator_feed(timing_accumulator_t *a, unsigned level, uint32_t ticks, unsigned limit);
esp_err_t measurements_init(void);
void measurements_index(index_reading_t *out);
bool measurements_last_activity(activity_reading_t *out, int64_t *age_us);
esp_err_t measurements_rpm(unsigned samples, rpm_reading_t *out);
esp_err_t measurements_activity(unsigned ms, activity_reading_t *out);
esp_err_t measurements_timing(unsigned samples, timing_reading_t *out);
