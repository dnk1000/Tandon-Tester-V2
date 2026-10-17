#include "power_monitor.h"
#include <math.h>
float ina226_bus_voltage(uint16_t raw) { return raw*0.00125f; }
float ina226_shunt_current(uint16_t raw) { return (int16_t)raw*0.0001f; }
bool power_rail_fault(rail_reading_t *r, float nominal)
{
    const float low=nominal==5.0f?4.75f:11.4f;
    const float high=nominal==5.0f?5.25f:12.6f;
    r->under_voltage=r->valid && r->voltage<low;
    r->over_voltage=r->valid && r->voltage>high;
    r->over_current=r->valid && (fabsf(r->current)>2.0f || r->current < -0.05f);
    return !r->valid || r->under_voltage || r->over_voltage || r->over_current || r->alert || r->overflow;
}
