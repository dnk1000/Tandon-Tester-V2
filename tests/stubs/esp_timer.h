/**
 * @file esp_timer.h
 * @brief Host-only esp timer fixture or declaration; not a physical hardware validation.
 */
#pragma once
#include <stdint.h>
/**
 * @brief Return virtual microseconds for HAL timing tests.
 */
int64_t esp_timer_get_time(void);
