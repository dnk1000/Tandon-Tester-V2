# @file CMakeLists.txt
# @brief Firmware sources and explicit ESP-IDF component dependencies.
#
idf_component_register(SRCS "app_main.c" "cli.c" "command.c" "drive.c" "floppy_hal.c"
    "measurements.c" "timing.c" "power_monitor.c" "power_values.c" "config.c" "config_values.c" "diagnostics.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_rmt esp_driver_pcnt
             esp_driver_usb_serial_jtag vfs esp_app_format esp_timer nvs_flash esp_system)
