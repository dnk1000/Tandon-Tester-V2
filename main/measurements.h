/**
 * @file measurements.h
 * @brief Index, PCNT activity and RMT DMA timing measurements.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/** @brief Index result: timing in microseconds, rising-edge count and freshness. */
typedef struct {
    bool active, valid;
    int64_t last_rise, period_us, width_us, min_us, max_us;
    uint32_t pulses;
} index_reading_t;
/** @brief RPM result: speed in RPM, deviation in percent, timing/jitter in microseconds. */
typedef struct {
    unsigned samples;
    double rpm, rpm_min, rpm_max, deviation, period_min_us, period_max_us, jitter_us;
    bool passed;
} rpm_reading_t;
/** @brief Activity result: rising-edge count, actual microseconds and pulses/second. */
typedef struct { uint32_t pulses; int64_t duration_us; double rate; } activity_reading_t;
/** @brief Timing result: 50 ns ticks and 1 us histogram bins; bin 15 means >=15 us. */
typedef struct {
    uint32_t samples, min_ticks, max_ticks, histogram[16];
    uint64_t sum_ticks;
} timing_reading_t;
// Portable streaming rising-edge interval reducer, also exercised by host tests.
/** @brief Reducer state preserved across consecutive DMA segments of one frame. */
typedef struct {
    timing_reading_t result;
    uint64_t elapsed, last_rise;
    int previous_level;
    bool have_rise;
} timing_accumulator_t;
/**
 * @brief Reset the interval reducer before a new contiguous capture.
 *
 * @param a Non-NULL streaming accumulator retaining state across contiguous segments.
 */
void timing_accumulator_init(timing_accumulator_t *a);
/**
 * @brief Consume one duration while preserving rising-edge state across DMA blocks.
 *
 * @param a Non-NULL streaming accumulator retaining state across contiguous segments.
 * @param level Buffered signal level, 0 or 1.
 * @param ticks Duration in 50 ns ticks; zero denotes an ignored end marker.
 * @param limit Maximum complete intervals to retain.
 */
void timing_accumulator_feed(timing_accumulator_t *a, unsigned level, uint32_t ticks, unsigned limit);
/**
 * @brief Allocate index, PCNT and RMT resources once; enable APIs only on full success.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t measurements_init(void);
/**
 * @brief Copy index statistics under lock and recompute one-second freshness.
 *
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 */
void measurements_index(index_reading_t *out);
/**
 * @brief Read the last successful activity result rather than a continuous live count.
 *
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @param age_us Non-NULL age destination in microseconds, or -1 before the first observation.
 * @return true when a previous successful observation exists.
 */
bool measurements_last_activity(activity_reading_t *out, int64_t *age_us);
/**
 * @brief Measure new complete index intervals with bounded cancellable waits.
 *
 * @param samples Complete index intervals (1–100) or read intervals (1–10000), according to the API.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for complete acquisition; inspect passed separately for speed tolerance.
 */
esp_err_t measurements_rpm(unsigned samples, rpm_reading_t *out);
/**
 * @brief Count read assertion edges with PCNT over a bounded observation window.
 *
 * @param ms Requested milliseconds; scheduler delays may extend elapsed time.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for a complete window, including zero pulses; caller classifies absence.
 */
esp_err_t measurements_activity(unsigned ms, activity_reading_t *out);
/**
 * @brief Capture a contiguous RMT frame and summarize complete rising-edge intervals.
 *
 * @param samples Complete index intervals (1–100) or read intervals (1–10000), according to the API.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for the requested count; partial data is returned on timeout.
 */
esp_err_t measurements_timing(unsigned samples, timing_reading_t *out);
