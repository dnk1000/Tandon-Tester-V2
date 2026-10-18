/**
 * @file drive.c
 * @brief Worker-owned drive state, bounded motion and centrally enforced timing.
 */
#include "drive.h"
#include <stdatomic.h>
#include "config.h"
#include "floppy_hal.h"
#include "power_monitor.h"
static drive_state_t state;
static atomic_bool cancelled;
// Separate latch: a later command cannot erase an unhandled stop request.
static atomic_bool stop_pending;
static int64_t last_step;
static bool have_step;
/**
 * @brief Refresh positive-logic sensor state without changing inferred position validity.
 */
static void sample_sensors(void)
{
    state.track0=floppy_hal_track0();
    state.write_protected=floppy_hal_wp();
}
/**
 * @brief Initialize deasserted outputs and unknown position before console tasks start.
 */
void drive_init(void) { drive_stop(); have_step=false; atomic_store(&stop_pending,false); }
/**
 * @brief Borrow worker-owned drive state and its most recently sampled sensors.
 * @return Borrowed state; track is meaningful only when position_valid is true.
 */
const drive_state_t *drive_state(void) { return &state; }
/**
 * @brief Release controls and discard inferred head position.
 */
void drive_stop(void)
{
    floppy_hal_safe();
    state=(drive_state_t){.selected=-1, .track=-1};
    sample_sensors();
}
/**
 * @brief Atomically request cancellation and a persistent stop without changing GPIOs.
 */
void drive_cancel(void)
{
    atomic_store(&stop_pending,true);
    atomic_store(&cancelled,true);
}
/**
 * @brief Clear active cancellation while retaining an unconsumed stop request.
 */
void drive_clear_cancel(void) { atomic_store(&cancelled,false); }
/**
 * @brief Query the thread-safe cancellation flag.
 * @return true after a cancellation request until its active flag is cleared.
 */
bool drive_cancelled(void) { return atomic_load(&cancelled); }
/**
 * @brief Refresh sensors and release controls on cancellation or unhealthy power.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
static esp_err_t guard(void)
{
    sample_sensors();
    if (drive_cancelled() || !power_monitor_healthy()) {
        drive_stop();
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}
/**
 * @brief Consume pending stops and enforce power and Track 0 consistency while idle.
 */
void drive_poll(void)
{
    sample_sensors();
    if (atomic_exchange(&stop_pending,false) || drive_cancelled() || !power_monitor_healthy() ||
        (state.position_valid && floppy_hal_track0() != (state.track==0))) drive_stop();
}
/**
 * @brief Wait in bounded slices while checking cancellation, sensors and power.
 *
 * @param ms Requested milliseconds; scheduler delays may extend elapsed time.
 * @return ESP_OK after the requested wait, or an interlock error with controls released.
 */
esp_err_t drive_wait_ms(uint32_t ms)
{
    int64_t end=floppy_hal_now_us()+(int64_t)ms*1000;
    do {
        esp_err_t e=guard();
        if (e!=ESP_OK) return e;
        int64_t left=end-floppy_hal_now_us();
        if (left<=0) break;
        floppy_hal_delay_us(left>5000?5000:(uint32_t)left);
    } while (true);
    return ESP_OK;
}
/**
 * @brief Select one drive with break-before-make; invalidate position on a change.
 *
 * @param ds Drive-select index 0–3; HAL releases every select for any other value.
 * @return ESP_OK after settling; a range or power/cancellation error otherwise.
 */
esp_err_t drive_select(unsigned ds)
{
    if (ds>3) return ESP_ERR_INVALID_ARG;
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    if (state.selected==(int)ds) return ESP_OK;
    drive_stop(); // break-before-make, no position transfers between drives
    floppy_hal_select(ds);
    state.selected=ds;
    return drive_wait_ms(1); // conservative select settling
}
/**
 * @brief Release all controls, including the motor, and invalidate position.
 */
void drive_deselect(void) { drive_stop(); }
/**
 * @brief Assert motor independently of selection and wait the configured spin-up delay.
 * @return ESP_OK after the spin-up delay or if already enabled; an interlock error otherwise.
 */
esp_err_t motor_on(void)
{
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    if (state.motor) return ESP_OK;
    floppy_hal_motor(true);
    state.motor=true;
    return drive_wait_ms(config_get()->motor_spinup_ms);
}
/**
 * @brief Release the motor without discarding an otherwise known head position.
 */
void motor_off(void) { floppy_hal_motor(false); state.motor=false; }
/**
 * @brief Generate one direction-qualified pulse with enforced spacing and settling.
 *
 * @param inward true toward higher tracks; false toward Track 0.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
static esp_err_t pulse(bool inward)
{
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    if (state.selected<0) return ESP_ERR_INVALID_STATE;
    uint32_t interval=config_get()->step_time*1000;
    // Defend absolute hardware minimum even if a caller supplies corrupt configuration.
    if (interval<5000) interval=5000;
    while (have_step && floppy_hal_now_us()-last_step<interval) {
        e=drive_wait_ms(1);
        if (e!=ESP_OK) return e;
    }
    floppy_hal_direction(inward);
    state.inward=inward;
// Preserve pulse history across stops/selections to prevent timing bypass.
    last_step=floppy_hal_now_us();
    have_step=true;
    floppy_hal_step_pulse();
    // Wait for mechanism/sensor settling before examining Track 0 or returning.
    return drive_wait_ms(config_get()->step_time<5?5:config_get()->step_time);
}
/**
 * @brief Compare Track 0 with the inferred track before accepting position.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
static esp_err_t check_position(void)
{
    if (!state.position_valid) return ESP_ERR_INVALID_STATE;
    if (floppy_hal_track0() != (state.track==0)) {
        drive_stop();
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}
/**
 * @brief Move one legal track from a homed position and check Track 0 after settling.
 *
 * @param inward true toward higher tracks; false toward Track 0.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
static esp_err_t step(bool inward)
{
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    if (state.selected<0 || !state.position_valid) return ESP_ERR_INVALID_STATE;
    e=check_position();
    if (e!=ESP_OK) return e;
    if ((inward && state.track>=(int)config_get()->max_track) ||
        (!inward && state.track<=0)) return ESP_ERR_INVALID_ARG;
    e=pulse(inward);
    if (e!=ESP_OK) { drive_stop(); return e; }
    state.track+=inward?1:-1;
    return check_position();
}
/**
 * @brief Move one track inward without exceeding the configured maximum.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t step_in(void) { return step(true); }
/**
 * @brief Move one track outward without stepping below Track 0.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t step_out(void) { return step(false); }
/**
 * @brief Auto-select if needed and restore toward Track 0 within a hard pulse limit.
 * @return ESP_OK only after stable Track 0; timeout or interlock error otherwise.
 */
esp_err_t home(void)
{
    state.position_valid=false;
    state.track=-1;
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    if (state.selected<0) {
        e=drive_select(config_get()->drive_select);
        if (e!=ESP_OK) return e;
    }
    floppy_hal_direction(false);
    state.inward=false;
// Check after the last legal pulse, but never issue pulse home_limit+1.
    for (unsigned i=0; i<=config_get()->home_limit; ++i) {
        if (floppy_hal_track0()) {
            e=drive_wait_ms(2); // reject a transient Track 0 indication
            if (e!=ESP_OK) return e;
            if (floppy_hal_track0()) {
                state.track=0; state.position_valid=true;
                return ESP_OK;
            }
        }
        if (i==config_get()->home_limit) break;
        e=pulse(false);
        if (e!=ESP_OK) { drive_stop(); return e; }
    }
    drive_stop();
    return ESP_ERR_TIMEOUT;
}
/**
 * @brief Move to a legal target only after successful homing has established position.
 *
 * @param track Inclusive target from 0 through configured max_track.
 * @return ESP_OK on completion; invalid position, range, sensor or interlock error otherwise.
 */
esp_err_t seek(unsigned track)
{
    if (track>config_get()->max_track) return ESP_ERR_INVALID_ARG;
    esp_err_t e=guard();
    if (e!=ESP_OK) return e;
    e=check_position();
    if (e!=ESP_OK) return e;
    while (state.track!=(int)track) {
        e=step(state.track<(int)track);
        if (e!=ESP_OK) return e;
    }
    return ESP_OK;
}
