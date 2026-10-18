/**
 * @file power_monitor.h
 * @brief Dual INA226 sampling, coherent rail snapshots and power interlock.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/**
 * @brief Rail voltage/current in volts/amperes with diagnostic flags.
 * @details Numeric readings are meaningful only when valid is true. error records
 * setup/I2C failures; alert and overflow reflect INA226 device flags.
 */
typedef struct {
    float voltage, current;
    bool valid, under_voltage, over_voltage, over_current, alert, overflow;
    esp_err_t error;
} rail_reading_t;
/** @brief Coherent dual-rail snapshot with positive fault semantics and publication time in us. */
typedef struct { rail_reading_t rail5, rail12; bool fault_pin; int64_t timestamp; } power_reading_t;
/**
 * @brief Create I2C device handles and start asynchronous identification and sampling.
 * @return ESP_OK when the task starts; this alone does not establish healthy rails.
 */
esp_err_t power_monitor_init(void);
/**
 * @brief Copy both rails atomically relative to the monitor task.
 *
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 */
void power_monitor_snapshot(power_reading_t *out);
/**
 * @brief Require fresh valid rails and a deasserted live hardware fault input.
 * @return true only for healthy rails published less than 500 ms ago and no live alert.
 */
bool power_monitor_healthy(void);
// Pure conversions shared by device code and host tests.
/**
 * @brief Scale unsigned bus counts at 1.25 mV per count.
 *
 * @param raw Raw INA226 register bits; shunt conversion reinterprets the value as signed 16-bit.
 * @return Bus voltage in volts.
 */
float ina226_bus_voltage(uint16_t raw);
/**
 * @brief Scale signed shunt counts using 2.5 uV/count and a 25 mOhm resistor.
 *
 * @param raw Raw INA226 register bits; shunt conversion reinterprets the value as signed 16-bit.
 * @return Shunt current in amperes, including signed reverse current.
 */
float ina226_shunt_current(uint16_t raw);
/**
 * @brief Classify supported rail readings against voltage/current and device fault limits.
 *
 * @param rail Non-NULL reading updated with diagnostic voltage/current flags.
 * @param nominal 5.0 selects +5 V limits; the other supported caller supplies 12.0.
 * @return true for invalid data or any fault; software detection does not disconnect power.
 */
bool power_rail_fault(rail_reading_t *rail, float nominal);
