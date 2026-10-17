#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
// HDS v1.0 sections 6–8. LS06 outputs and LVC14 inputs are MCU-active-high.
enum {
    PIN_DS0=4, PIN_DS1=5, PIN_DS2=6, PIN_DS3=7,
    PIN_I2C_SDA=8, PIN_I2C_SCL=9, PIN_MOTOR=10, PIN_DIR=11, PIN_STEP=12,
    PIN_WRITE_DATA=13, PIN_WRITE_GATE=14, PIN_SIDE=15,
    PIN_INDEX=16, PIN_TRACK0=17, PIN_WP=18, PIN_READ_DATA=21,
    PIN_POWER_FAULT=38, PIN_HW_WRITE_ENABLE=39,
    PIN_LED_STATUS=40, PIN_LED_FAULT=41, PIN_LED_WRITE_ARMED=42
};
esp_err_t floppy_hal_init(void);
void floppy_hal_safe(void);
void floppy_hal_select(int ds); // -1 deasserts every select
void floppy_hal_motor(bool on);
void floppy_hal_direction(bool inward);
void floppy_hal_step_pulse(void); // 5 us pulse, >=10 us direction setup/hold
void floppy_hal_side(bool side1);
bool floppy_hal_track0(void);
bool floppy_hal_wp(void);
bool floppy_hal_index(void);
bool floppy_hal_power_fault(void);
bool floppy_hal_hw_write_enabled(void);
void floppy_hal_fault_led(bool on);
int64_t floppy_hal_now_us(void);
void floppy_hal_delay_us(uint32_t us);
// Deliberately no Write Gate/Data assertion API in Revision A.
