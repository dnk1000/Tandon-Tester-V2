/**
 * @file drive.h
 * @brief Worker-owned drive state, bounded motion and centrally enforced timing.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/**
 * @brief Worker-owned inferred mechanism state and latest sampled inputs.
 * @details selected=-1 is deselected; track=-1 is unknown. Use track only when
 * position_valid is true. It is inferred from commands and Track 0, not an encoder.
 */
typedef struct { int selected, track; bool motor, inward, position_valid, track0, write_protected; } drive_state_t;
/**
 * @brief Initialize deasserted outputs and unknown position before console tasks start.
 */
void drive_init(void);
/**
 * @brief Borrow worker-owned drive state and its most recently sampled sensors.
 * @return Borrowed state; track is meaningful only when position_valid is true.
 */
const drive_state_t *drive_state(void); // single command-worker ownership
/**
 * @brief Release controls and discard inferred head position.
 */
void drive_stop(void); // all outputs safe, position invalid
/**
 * @brief Consume pending stops and enforce power and Track 0 consistency while idle.
 */
void drive_poll(void); // worker idle safety check
/**
 * @brief Atomically request cancellation and a persistent stop without changing GPIOs.
 */
void drive_cancel(void); // thread-safe request, never changes GPIO from input task
/**
 * @brief Clear active cancellation while retaining an unconsumed stop request.
 */
void drive_clear_cancel(void);
/**
 * @brief Query the thread-safe cancellation flag.
 * @return true after a cancellation request until its active flag is cleared.
 */
bool drive_cancelled(void);
/**
 * @brief Wait in bounded slices while checking cancellation, sensors and power.
 *
 * @param ms Requested milliseconds; scheduler delays may extend elapsed time.
 * @return ESP_OK after the requested wait, or an interlock error with controls released.
 */
esp_err_t drive_wait_ms(uint32_t ms); // cancellable; checks rail health
/**
 * @brief Select one drive with break-before-make; invalidate position on a change.
 *
 * @param ds Drive-select index 0–3; HAL releases every select for any other value.
 * @return ESP_OK after settling; a range or power/cancellation error otherwise.
 */
esp_err_t drive_select(unsigned ds);
/**
 * @brief Release all controls, including the motor, and invalidate position.
 */
void drive_deselect(void);
/**
 * @brief Assert motor independently of selection and wait the configured spin-up delay.
 * @return ESP_OK after the spin-up delay or if already enabled; an interlock error otherwise.
 */
esp_err_t motor_on(void);
/**
 * @brief Release the motor without discarding an otherwise known head position.
 */
void motor_off(void);
/**
 * @brief Move one track inward without exceeding the configured maximum.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t step_in(void);
/**
 * @brief Move one track outward without stepping below Track 0.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t step_out(void);
/**
 * @brief Auto-select if needed and restore toward Track 0 within a hard pulse limit.
 * @return ESP_OK only after stable Track 0; timeout or interlock error otherwise.
 */
esp_err_t home(void);
/**
 * @brief Move to a legal target only after successful homing has established position.
 *
 * @param track Inclusive target from 0 through configured max_track.
 * @return ESP_OK on completion; invalid position, range, sensor or interlock error otherwise.
 */
esp_err_t seek(unsigned track);
