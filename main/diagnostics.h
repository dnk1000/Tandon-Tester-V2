#pragma once
#include "esp_err.h"
esp_err_t diagnostics_basic(void);
esp_err_t diagnostics_seek(void);
esp_err_t diagnostics_exercise(unsigned cycles);
