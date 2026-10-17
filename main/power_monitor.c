#include "power_monitor.h"
#include <math.h>
#include "driver/i2c_master.h"
#include "floppy_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t devices[2]; // +12 U6 0x40, +5 U7 0x41
static power_reading_t latest={.rail5={.error=ESP_ERR_INVALID_STATE},
    .rail12={.error=ESP_ERR_INVALID_STATE}};
static bool ready[2];
static portMUX_TYPE lock=portMUX_INITIALIZER_UNLOCKED;
static esp_err_t read_reg(i2c_master_dev_handle_t d, uint8_t reg, uint16_t *v)
{
    uint8_t bytes[2];
    esp_err_t e=i2c_master_transmit_receive(d,&reg,1,bytes,2,50);
    if (e==ESP_OK) *v=((uint16_t)bytes[0]<<8)|bytes[1];
    return e;
}
static esp_err_t write_reg(i2c_master_dev_handle_t d, uint8_t reg, uint16_t v)
{
    uint8_t bytes[]={reg,v>>8,v&0xff};
    return i2c_master_transmit(d,bytes,3,50);
}
static esp_err_t setup_device(unsigned i)
{
    uint16_t id;
    esp_err_t e=read_reg(devices[i],0xfe,&id);
    if (e!=ESP_OK) return e;
    if (id!=0x5449) return ESP_ERR_INVALID_RESPONSE;
    e=read_reg(devices[i],0xff,&id);
    if (e!=ESP_OK) return e;
    if ((id&0xfff0)!=0x2260) return ESP_ERR_INVALID_RESPONSE;
    // Four averages, 1.1ms shunt/bus conversions, continuous shunt+bus mode.
    e=write_reg(devices[i],0,0x4327);
    if (e!=ESP_OK) return e;
    // 100uA/bit current register, 25mOhm: CAL=0.00512/(0.0001*0.025)=2048.
    e=write_reg(devices[i],5,2048);
    if (e!=ESP_OK) return e;
    // Shunt-overvoltage alert at 50mV (2A); active-low, non-latching open drain.
    e=write_reg(devices[i],7,20000);
    if (e!=ESP_OK) return e;
    return write_reg(devices[i],6,1u<<15);
}
static rail_reading_t sample(unsigned i)
{
    rail_reading_t r={0};
    uint16_t v, s, mask, cal;
    esp_err_t e=read_reg(devices[i],2,&v);
    if (e==ESP_OK) e=read_reg(devices[i],1,&s);
    if (e==ESP_OK) e=read_reg(devices[i],6,&mask);
    if (e==ESP_OK) e=read_reg(devices[i],5,&cal);
    if (e==ESP_OK && cal!=2048) e=ESP_ERR_INVALID_STATE; // detect monitor reset
    r.error=e;
    if (e==ESP_OK) {
        r.voltage=ina226_bus_voltage(v);
        // Direct signed shunt conversion avoids relying on calibrated current rounding.
        r.current=ina226_shunt_current(s);
        r.valid=true;
        r.alert=mask&(1u<<4);
        r.overflow=mask&(1u<<2);
    }
    power_rail_fault(&r,i==0?12.0f:5.0f);
    return r;
}
static bool rail_healthy(const rail_reading_t *r)
{
    return r->valid && !r->under_voltage && !r->over_voltage && !r->over_current && !r->alert && !r->overflow;
}
static void monitor_task(void *arg)
{
    (void)arg;
    for (;;) {
        rail_reading_t readings[2]={0};
        for (unsigned i=0;i<2;++i) {
            if (!ready[i]) {
                readings[i].error=setup_device(i);
                ready[i]=readings[i].error==ESP_OK;
                if (ready[i]) readings[i].error=ESP_ERR_INVALID_STATE;
                // New conversions need one complete average; don't publish reset data as valid.
            } else {
                readings[i]=sample(i);
                if (!readings[i].valid) ready[i]=false;
            }
        }
        power_reading_t p={.rail12=readings[0], .rail5=readings[1],
            .fault_pin=floppy_hal_power_fault(), .timestamp=floppy_hal_now_us()};
        portENTER_CRITICAL(&lock); latest=p; portEXIT_CRITICAL(&lock);
        floppy_hal_fault_led(!rail_healthy(&p.rail5)||!rail_healthy(&p.rail12)||p.fault_pin);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
esp_err_t power_monitor_init(void)
{
    i2c_master_bus_config_t c={.i2c_port=-1,.sda_io_num=PIN_I2C_SDA,
        .scl_io_num=PIN_I2C_SCL,.clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,
        .flags.enable_internal_pullup=true};
    esp_err_t e=i2c_new_master_bus(&c,&bus);
    if (e!=ESP_OK) return e;
    for (unsigned i=0;i<2;++i) {
        i2c_device_config_t d={.dev_addr_length=I2C_ADDR_BIT_LEN_7,.device_address=0x40+i,.scl_speed_hz=100000};
        e=i2c_master_bus_add_device(bus,&d,&devices[i]);
        if (e!=ESP_OK) return e;
    }
    return xTaskCreate(monitor_task,"power",4096,NULL,5,NULL)==pdPASS?ESP_OK:ESP_ERR_NO_MEM;
}
void power_monitor_snapshot(power_reading_t *out)
{
    portENTER_CRITICAL(&lock); *out=latest; portEXIT_CRITICAL(&lock);
}
bool power_monitor_healthy(void)
{
    power_reading_t p;
    power_monitor_snapshot(&p);
    return p.timestamp>0 && floppy_hal_now_us()-p.timestamp<500000 &&
        !floppy_hal_power_fault() && !p.fault_pin && rail_healthy(&p.rail5) && rail_healthy(&p.rail12);
}
