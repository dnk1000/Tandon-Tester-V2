/**
 * @file test_hal.c
 * @brief Host-only test hal fixture or declaration; not a physical hardware validation.
 */
#include <assert.h>
#include <stdio.h>
#include "floppy_hal.h"
#include "driver/gpio.h"
static int levels[49];
static uint64_t preloaded, outputs;
static int64_t clock_us,last_dir,step_start;
static unsigned step_count;
/**
 * @brief Record GPIO writes and assert that write-related outputs never go HIGH.
 */
esp_err_t gpio_set_level(int pin,int value)
{
    assert(pin>=0 && pin<49 && pin!=19 && pin!=20); // USB never touched
    if (pin==PIN_WRITE_GATE || pin==PIN_WRITE_DATA || pin==PIN_LED_WRITE_ARMED) assert(value==0);
    if (pin>=PIN_DS0 && pin<=PIN_DS3 && value)
        for (int i=PIN_DS0;i<=PIN_DS3;++i) if (i!=pin) assert(!levels[i]);
    if (pin==PIN_DIR) last_dir=clock_us;
    if (pin==PIN_STEP && value) { assert(clock_us-last_dir>=10);step_start=clock_us;++step_count; }
    if (pin==PIN_STEP && !value && levels[pin]) assert(clock_us-step_start>=5);
    levels[pin]=value;preloaded|=1ULL<<pin;return ESP_OK;
}
/**
 * @brief Verify output latch preloading before recording output enable.
 */
esp_err_t gpio_config(const gpio_config_t *c)
{
    if (c->mode==GPIO_MODE_OUTPUT) {
        assert((preloaded&c->pin_bit_mask)==c->pin_bit_mask); // latch loaded before output enable
        for (int i=0;i<49;++i) if ((1ULL<<i)&c->pin_bit_mask) assert(!levels[i]);
        outputs|=c->pin_bit_mask;
    }
    return ESP_OK;
}
/**
 * @brief Read the fixture GPIO level without hardware access.
 */
int gpio_get_level(int pin) { return levels[pin]; }
/**
 * @brief Advance virtual time instead of busy-waiting on the host.
 */
void esp_rom_delay_us(unsigned us) { clock_us+=us; }
/**
 * @brief Advance deterministic fixture time without running a scheduler.
 */
void vTaskDelay(unsigned ticks) { clock_us+=ticks*1000; }
/**
 * @brief Return virtual microseconds for HAL timing tests.
 */
int64_t esp_timer_get_time(void) { return clock_us; }
/**
 * @brief Run the hal host fixture assertions.
 */
int main(void)
{
    assert(floppy_hal_init()==ESP_OK);
    uint64_t required=(1ULL<<4)|(1ULL<<5)|(1ULL<<6)|(1ULL<<7)|(1ULL<<10)|(1ULL<<11)|
        (1ULL<<12)|(1ULL<<13)|(1ULL<<14)|(1ULL<<15);
    assert((outputs&required)==required);
    for (int i=0;i<4;++i) { floppy_hal_select(i);assert(levels[PIN_DS0+i]); }
    floppy_hal_select(-1);for (int i=0;i<4;++i) assert(!levels[PIN_DS0+i]);
    floppy_hal_motor(true);assert(levels[PIN_MOTOR]);
    floppy_hal_direction(true);floppy_hal_step_pulse();assert(step_count==1 && !levels[PIN_STEP]);
    assert(clock_us-step_start>=15);floppy_hal_side(true);assert(levels[PIN_SIDE]);
    floppy_hal_safe();for (int i=0;i<49;++i) if (required&(1ULL<<i)) assert(!levels[i]);
    levels[PIN_TRACK0]=1;levels[PIN_WP]=1;levels[PIN_INDEX]=1;levels[PIN_HW_WRITE_ENABLE]=1;
    assert(floppy_hal_track0()&&floppy_hal_wp()&&floppy_hal_index()&&floppy_hal_hw_write_enabled());
    levels[PIN_POWER_FAULT]=0;assert(floppy_hal_power_fault());
    levels[PIN_POWER_FAULT]=1;assert(!floppy_hal_power_fault());
    puts("PASS: actual HAL initialization, break-before-make, inversion, timing, safe outputs; writes always LOW");
    return 0;
}
