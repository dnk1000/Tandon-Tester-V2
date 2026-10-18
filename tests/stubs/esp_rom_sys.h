/**
 * @file esp_rom_sys.h
 * @brief Host-only esp rom sys fixture or declaration; not a physical hardware validation.
 */
#pragma once
/**
 * @brief Advance virtual time instead of busy-waiting on the host.
 */
void esp_rom_delay_us(unsigned us);
