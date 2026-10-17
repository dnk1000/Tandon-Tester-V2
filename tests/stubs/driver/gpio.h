#pragma once
#include <stdint.h>
#include "esp_err.h"
#define GPIO_MODE_OUTPUT 1
#define GPIO_MODE_INPUT 2
#define GPIO_PULLDOWN_ENABLE 1
#define GPIO_PULLUP_ENABLE 1
typedef struct { uint64_t pin_bit_mask; int mode,pull_down_en,pull_up_en; } gpio_config_t;
esp_err_t gpio_set_level(int pin,int level);
esp_err_t gpio_config(const gpio_config_t *c);
int gpio_get_level(int pin);
