#include "floppy_hal.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const int outputs[] = {PIN_WRITE_GATE, PIN_WRITE_DATA, PIN_STEP, PIN_MOTOR,
    PIN_DS0, PIN_DS1, PIN_DS2, PIN_DS3, PIN_DIR, PIN_SIDE,
    PIN_LED_STATUS, PIN_LED_FAULT, PIN_LED_WRITE_ARMED};
static const int selects[] = {PIN_DS0, PIN_DS1, PIN_DS2, PIN_DS3};
static portMUX_TYPE pulse_lock = portMUX_INITIALIZER_UNLOCKED;
esp_err_t floppy_hal_init(void)
{
    // Preload LOW before enabling output drivers; external 10k pull-downs cover ROM/reset.
    for (unsigned i=0; i<sizeof(outputs)/sizeof(outputs[0]); ++i) {
        gpio_set_level(outputs[i], 0);
        gpio_config_t c = {.pin_bit_mask=1ULL<<outputs[i], .mode=GPIO_MODE_OUTPUT,
            .pull_down_en=GPIO_PULLDOWN_ENABLE};
        esp_err_t e=gpio_config(&c);
        if (e != ESP_OK) return e;
    }
    gpio_config_t c = {.pin_bit_mask=(1ULL<<PIN_INDEX)|(1ULL<<PIN_TRACK0)|
        (1ULL<<PIN_WP)|(1ULL<<PIN_READ_DATA)|(1ULL<<PIN_HW_WRITE_ENABLE),
        .mode=GPIO_MODE_INPUT, .pull_down_en=GPIO_PULLDOWN_ENABLE};
    esp_err_t e=gpio_config(&c);
    if (e != ESP_OK) return e;
    c = (gpio_config_t){.pin_bit_mask=1ULL<<PIN_POWER_FAULT, .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE};
    e=gpio_config(&c);
    floppy_hal_safe();
    return e;
}
void floppy_hal_safe(void)
{
    for (unsigned i=0; i<sizeof(outputs)/sizeof(outputs[0]); ++i) {
        if (outputs[i]!=PIN_LED_FAULT) gpio_set_level(outputs[i],0);
    }
}
void floppy_hal_select(int ds)
{
    for (int i=0;i<4;++i) gpio_set_level(selects[i],0);
    if (ds>=0 && ds<4) gpio_set_level(selects[ds],1);
    gpio_set_level(PIN_LED_STATUS, ds>=0 && ds<4);
}
void floppy_hal_motor(bool on) { gpio_set_level(PIN_MOTOR,on); }
void floppy_hal_direction(bool inward) { gpio_set_level(PIN_DIR,inward); }
void floppy_hal_side(bool side1) { gpio_set_level(PIN_SIDE,side1); }
void floppy_hal_step_pulse(void)
{
    esp_rom_delay_us(10);
    portENTER_CRITICAL(&pulse_lock);
    gpio_set_level(PIN_STEP,1);
    esp_rom_delay_us(5);
    gpio_set_level(PIN_STEP,0);
    portEXIT_CRITICAL(&pulse_lock);
    esp_rom_delay_us(10);
}
bool floppy_hal_track0(void) { return gpio_get_level(PIN_TRACK0); }
bool floppy_hal_wp(void) { return gpio_get_level(PIN_WP); }
bool floppy_hal_index(void) { return gpio_get_level(PIN_INDEX); }
bool floppy_hal_power_fault(void) { return !gpio_get_level(PIN_POWER_FAULT); }
bool floppy_hal_hw_write_enabled(void) { return gpio_get_level(PIN_HW_WRITE_ENABLE); }
void floppy_hal_fault_led(bool on) { gpio_set_level(PIN_LED_FAULT,on); }
int64_t floppy_hal_now_us(void) { return esp_timer_get_time(); }
void floppy_hal_delay_us(uint32_t us)
{
    if (us>=1000) vTaskDelay(pdMS_TO_TICKS((us+999)/1000));
    else esp_rom_delay_us(us);
}
