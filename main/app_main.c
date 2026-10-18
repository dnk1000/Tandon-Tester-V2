/**
 * @file app_main.c
 * @brief Safe startup ordering and subsystem diagnostics.
 */
#include <stdio.h>
#include "floppy_hal.h"
#include "drive.h"
#include "config.h"
#include "measurements.h"
#include "power_monitor.h"
#include "cli.h"
#include "esp_log.h"
/**
 * @brief Log failed subsystem startup without hiding unrelated diagnostic capabilities.
 *
 * @param name Subsystem label for logging.
 * @param e Startup result; ESP_OK is not logged.
 */
static void report(const char *name, esp_err_t e)
{
    if (e!=ESP_OK) ESP_LOGE("startup","%s: %s",name,esp_err_to_name(e));
}
/**
 * @brief Initialize safe outputs before configuration, measurements, power monitoring and USB.
 */
void app_main(void)
{
    // First application action: preload and deassert every floppy output.
    esp_err_t e=floppy_hal_init();
    if (e!=ESP_OK) { floppy_hal_safe(); report("HAL",e); return; }
    drive_init();
    report("NVS (defaults retained on error)",config_init());
    report("Measurements (commands fail closed on error)",measurements_init());
    report("INA226 (motion blocked until both monitors healthy)",power_monitor_init());
    report("USB CLI",cli_start());
}
