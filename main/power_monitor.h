#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct {
    float voltage, current;
    bool valid, under_voltage, over_voltage, over_current, alert, overflow;
    esp_err_t error;
} rail_reading_t;
typedef struct { rail_reading_t rail5, rail12; bool fault_pin; int64_t timestamp; } power_reading_t;
esp_err_t power_monitor_init(void);
void power_monitor_snapshot(power_reading_t *out);
bool power_monitor_healthy(void);
// Pure conversions shared by device code and host tests.
float ina226_bus_voltage(uint16_t raw);
float ina226_shunt_current(uint16_t raw);
bool power_rail_fault(rail_reading_t *rail, float nominal);
