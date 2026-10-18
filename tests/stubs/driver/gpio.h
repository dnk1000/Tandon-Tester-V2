/**
 * @file gpio.h
 * @brief Host-only gpio fixture or declaration; not a physical hardware validation.
 */
#pragma once
#include <stdint.h>
#include "esp_err.h"
#define GPIO_MODE_OUTPUT 1
#define GPIO_MODE_INPUT 2
#define GPIO_PULLDOWN_ENABLE 1
#define GPIO_PULLUP_ENABLE 1
typedef struct { uint64_t pin_bit_mask; int mode,pull_down_en,pull_up_en; } gpio_config_t;
/**
 * @brief Record GPIO writes and assert that write-related outputs never go HIGH.
 */
esp_err_t gpio_set_level(int pin,int level);
/**
 * @brief Verify output latch preloading before recording output enable.
 */
esp_err_t gpio_config(const gpio_config_t *c);
/**
 * @brief Read the fixture GPIO level without hardware access.
 */
int gpio_get_level(int pin);
