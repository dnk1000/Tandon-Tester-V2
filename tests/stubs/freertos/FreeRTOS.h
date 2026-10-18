/**
 * @file FreeRTOS.h
 * @brief Host-only FreeRTOS fixture or declaration; not a physical hardware validation.
 */
#pragma once
#include <stdint.h>
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(x) ((void)(x))
#define portEXIT_CRITICAL(x) ((void)(x))
#define pdMS_TO_TICKS(ms) (ms)
#define pdPASS 1
