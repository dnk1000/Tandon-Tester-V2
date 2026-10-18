/**
 * @file floppy_hal.h
 * @brief HDS v1.0 GPIO map, signal polarity and safe hardware controls.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
// HDS v1.0 sections 6–8. LS06 outputs and LVC14 inputs are MCU-active-high.
/**
 * @brief MCU GPIO numbers from HDS, not drive connector pin numbers.
 * @details LS06 output LOW releases the active-low external signal; buffered
 * input HIGH means asserted. GPIO19/20 remain exclusively native USB.
 * Write Data/Gate are defined but have no assertion API in Revision A.
 */
enum {
    PIN_DS0=4, PIN_DS1=5, PIN_DS2=6, PIN_DS3=7,
    PIN_I2C_SDA=8, PIN_I2C_SCL=9, PIN_MOTOR=10, PIN_DIR=11, PIN_STEP=12,
    PIN_WRITE_DATA=13, PIN_WRITE_GATE=14, PIN_SIDE=15,
    PIN_INDEX=16, PIN_TRACK0=17, PIN_WP=18, PIN_READ_DATA=21,
    PIN_POWER_FAULT=38, PIN_HW_WRITE_ENABLE=39,
    PIN_LED_STATUS=40, PIN_LED_FAULT=41, PIN_LED_WRITE_ARMED=42
};
/**
 * @brief Preload safe LOW latches before enabling GPIO outputs and configure inputs.
 * @return ESP_OK if setup succeeds, or the first GPIO configuration error.
 */
esp_err_t floppy_hal_init(void);
/**
 * @brief Release all controls while preserving the independently managed fault LED.
 */
void floppy_hal_safe(void);
/**
 * @brief Release every select before asserting at most one drive.
 *
 * @param ds Drive-select index 0–3; HAL releases every select for any other value.
 */
void floppy_hal_select(int ds); // -1 deasserts every select
/**
 * @brief Translate logical motor assertion to the LS06 input.
 *
 * @param on true to assert the logical motor/LED function.
 */
void floppy_hal_motor(bool on);
/**
 * @brief Set direction using the documented conventional Tandon electrical polarity.
 *
 * @param inward true toward higher tracks; false toward Track 0.
 */
void floppy_hal_direction(bool inward);
/**
 * @brief Generate a target 5 us pulse bracketed by at least 10 us setup and hold.
 */
void floppy_hal_step_pulse(void); // 5 us pulse, >=10 us direction setup/hold
/**
 * @brief Control the defined side-select driver; Revision A exposes no CLI assertion.
 *
 * @param side1 true asserts side select; Revision A normally keeps it false.
 */
void floppy_hal_side(bool side1);
/**
 * @brief Read asserted Track 0 through the active-HIGH buffer output.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_track0(void);
/**
 * @brief Read asserted write protection without enabling any write operation.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_wp(void);
/**
 * @brief Read the instantaneous buffered index pulse level.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_index(void);
/**
 * @brief Normalize active-LOW hardware ALERT to positive fault semantics.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_power_fault(void);
/**
 * @brief Observe physical switch sense without arming firmware writes.
 * @return true when the queried condition is asserted or valid.
 */
bool floppy_hal_hw_write_enabled(void);
/**
 * @brief Set the independent active-HIGH power fault indicator.
 *
 * @param on true to assert the logical motor/LED function.
 */
void floppy_hal_fault_led(bool on);
/**
 * @brief Read the monotonic microsecond clock used by software deadlines.
 * @return Monotonic microseconds from the ESP-IDF timer.
 */
int64_t floppy_hal_now_us(void);
/**
 * @brief Yield for millisecond waits or busy-wait for submillisecond delays.
 *
 * @param us Requested microseconds; scheduler delays may extend elapsed time.
 */
void floppy_hal_delay_us(uint32_t us);
// Deliberately no Write Gate/Data assertion API in Revision A.
