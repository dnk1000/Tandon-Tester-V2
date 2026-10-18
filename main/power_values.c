/**
 * @file power_values.c
 * @brief Portable INA226 unit conversion and fault classification.
 */
#include "power_monitor.h"
#include <math.h>
/**
 * @brief Scale unsigned bus counts at 1.25 mV per count.
 *
 * @param raw Raw INA226 register bits; shunt conversion reinterprets the value as signed 16-bit.
 * @return Bus voltage in volts.
 */
float ina226_bus_voltage(uint16_t raw) { return raw*0.00125f; }
/**
 * @brief Scale signed shunt counts using 2.5 uV/count and a 25 mOhm resistor.
 *
 * @param raw Raw INA226 register bits; shunt conversion reinterprets the value as signed 16-bit.
 * @return Shunt current in amperes, including signed reverse current.
 */
float ina226_shunt_current(uint16_t raw) { return (int16_t)raw*0.0001f; }
/**
 * @brief Classify supported rail readings against voltage/current and device fault limits.
 *
 * @param r Reading whose voltage/current flags are updated.
 * @param nominal 5.0 selects +5 V limits; the other supported caller supplies 12.0.
 * @return true for invalid data or any fault; software detection does not disconnect power.
 */
bool power_rail_fault(rail_reading_t *r, float nominal)
{
    const float low=nominal==5.0f?4.75f:11.4f;
    const float high=nominal==5.0f?5.25f:12.6f;
    r->under_voltage=r->valid && r->voltage<low;
    r->over_voltage=r->valid && r->voltage>high;
    r->over_current=r->valid && (fabsf(r->current)>2.0f || r->current < -0.05f);
    return !r->valid || r->under_voltage || r->over_voltage || r->over_current || r->alert || r->overflow;
}
