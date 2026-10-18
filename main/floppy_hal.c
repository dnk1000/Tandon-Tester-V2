/**
 * @file floppy_hal.c
 * @brief HDS v1.0 GPIO map, signal polarity and safe hardware controls.
 */
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
/**
 * @brief Preload safe LOW latches before enabling GPIO outputs and configure inputs.
 * @return ESP_OK if setup succeeds, or the first GPIO configuration error.
 */
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
/**
 * @brief Release all controls while preserving the independently managed fault LED.
 */
void floppy_hal_safe(void)
{
    for (unsigned i=0; i<sizeof(outputs)/sizeof(outputs[0]); ++i) {
// Motion cleanup must not hide the independently owned power fault LED.
        if (outputs[i]!=PIN_LED_FAULT) gpio_set_level(outputs[i],0);
    }
}
/**
 * @brief Release every select before asserting at most one drive.
 *
 * @param ds Drive-select index 0–3; HAL releases every select for any other value.
 */
void floppy_hal_select(int ds)
{
    for (int i=0;i<4;++i) gpio_set_level(selects[i],0);
    if (ds>=0 && ds<4) gpio_set_level(selects[ds],1);
    gpio_set_level(PIN_LED_STATUS, ds>=0 && ds<4);
}
/**
 * @brief Translate logical motor assertion to the LS06 input.
 *
 * @param on true to assert the logical motor/LED function.
 */
void floppy_hal_motor(bool on) { gpio_set_level(PIN_MOTOR,on); }
/**
 * @brief Set direction using the documented conventional Tandon electrical polarity.
 *
 * @param inward true toward higher tracks; false toward Track 0.
 */
void floppy_hal_direction(bool inward) { gpio_set_level(PIN_DIR,inward); }
/**
 * @brief Control the defined side-select driver; Revision A exposes no CLI assertion.
 *
 * @param side1 true asserts side select; Revision A normally keeps it false.
 */
void floppy_hal_side(bool side1) { gpio_set_level(PIN_SIDE,side1); }
/**
 * @brief Generate a target 5 us pulse bracketed by at least 10 us setup and hold.
 */
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
/**
 * @brief Read asserted Track 0 through the active-HIGH buffer output.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_track0(void) { return gpio_get_level(PIN_TRACK0); }
/**
 * @brief Read asserted write protection without enabling any write operation.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_wp(void) { return gpio_get_level(PIN_WP); }
/**
 * @brief Read the instantaneous buffered index pulse level.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_index(void) { return gpio_get_level(PIN_INDEX); }
/**
 * @brief Normalize active-LOW hardware ALERT to positive fault semantics.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_power_fault(void) { return !gpio_get_level(PIN_POWER_FAULT); }
/**
 * @brief Observe physical switch sense without arming firmware writes.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_hw_write_enabled(void) { return gpio_get_level(PIN_HW_WRITE_ENABLE); }
/**
 * @brief Set the independent active-HIGH power fault indicator.
 *
 * @param on true to assert the logical motor/LED function.
 */
void floppy_hal_fault_led(bool on) { gpio_set_level(PIN_LED_FAULT,on); }
/**
 * @brief Read the monotonic microsecond clock used by software deadlines.
 * @return Monotonic microseconds from the ESP-IDF timer.
 */
int64_t floppy_hal_now_us(void) { return esp_timer_get_time(); }
/**
 * @brief Yield for millisecond waits or busy-wait for submillisecond delays.
 *
 * @param us Requested microseconds; scheduler delays may extend elapsed time.
 */
void floppy_hal_delay_us(uint32_t us)
{
    if (us>=1000) vTaskDelay(pdMS_TO_TICKS((us+999)/1000));
    else esp_rom_delay_us(us);
}
