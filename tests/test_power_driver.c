#include <assert.h>
#include <string.h>
#include <stdio.h>
// Include production implementation to exercise register-level helpers without a scheduler.
#include "../main/power_monitor.c"
struct fake_device { uint16_t regs[256]; } fake[2];
static bool io_failure,pin_fault;
static int64_t now=1000000;
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *c,i2c_master_bus_handle_t *b)
{
    assert(c->sda_io_num==8 && c->scl_io_num==9); *b=fake;return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t b,const i2c_device_config_t *c,i2c_master_dev_handle_t *d)
{
    assert(b==fake && c->dev_addr_length==7 && c->scl_speed_hz==100000);
    assert(c->device_address==0x40 || c->device_address==0x41);
    *d=&fake[c->device_address-0x40];return ESP_OK;
}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t d,const uint8_t *bytes,size_t n,int timeout)
{
    assert(n==3 && timeout==50); if (io_failure) return ESP_ERR_TIMEOUT;
    d->regs[bytes[0]]=((uint16_t)bytes[1]<<8)|bytes[2];return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t d,const uint8_t *r,size_t rn,uint8_t *out,size_t n,int timeout)
{
    assert(rn==1 && n==2 && timeout==50);if (io_failure)return ESP_ERR_TIMEOUT;
    uint16_t val=d->regs[*r];out[0]=val>>8;out[1]=val&255;return ESP_OK;
}
int xTaskCreate(void (*task)(void *),const char *name,unsigned stack,void *arg,unsigned priority,void *handle)
{
    (void)task;(void)name;(void)stack;(void)arg;(void)priority;(void)handle;return pdPASS;
}
void vTaskDelay(unsigned ticks) { now+=ticks*1000; }
bool floppy_hal_power_fault(void) { return pin_fault; }
int64_t floppy_hal_now_us(void) { return now; }
void floppy_hal_fault_led(bool on) { (void)on; }
int main(void)
{
    assert(power_monitor_init()==ESP_OK);
    assert(!power_monitor_healthy());
    for (unsigned i=0;i<2;++i) {
        fake[i].regs[0xfe]=0x5449;fake[i].regs[0xff]=0x2260;
        assert(setup_device(i)==ESP_OK);
        assert(fake[i].regs[0]==0x4327 && fake[i].regs[5]==2048);
        assert(fake[i].regs[6]==0x8000 && fake[i].regs[7]==20000);
        fake[i].regs[2]=i?4000:9600;
        fake[i].regs[1]=i?6000:9000;
        rail_reading_t r=sample(i);assert(r.valid && r.error==ESP_OK && !power_rail_fault(&r,i?5:12));
    }
    latest=(power_reading_t){.rail12=sample(0),.rail5=sample(1),.timestamp=now};
    assert(power_monitor_healthy());now+=500001;assert(!power_monitor_healthy());
    latest.timestamp=now;pin_fault=true;assert(!power_monitor_healthy());pin_fault=false;
    fake[0].regs[5]=0;assert(!sample(0).valid);assert(setup_device(0)==ESP_OK);
    fake[0].regs[6]|=1<<4;assert(sample(0).alert);fake[0].regs[6]=1<<2;assert(sample(0).overflow);
    fake[0].regs[0xfe]=0;assert(setup_device(0)==ESP_ERR_INVALID_RESPONSE);
    io_failure=true;assert(setup_device(1)==ESP_ERR_TIMEOUT && !sample(1).valid);
    puts("PASS: INA226 addresses, identity, endian register writes/reads, reset detection, errors, freshness and fault interlock");
    return 0;
}
