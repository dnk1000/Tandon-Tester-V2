/**
 * @file measurements.c
 * @brief Index, PCNT activity and RMT DMA timing measurements.
 */
#include "measurements.h"
#include <stdatomic.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include "floppy_hal.h"
#include "drive.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "driver/pulse_cnt.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
static index_reading_t index_state;
static portMUX_TYPE index_lock=portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t index_queue;
static atomic_bool index_overflow;
static pcnt_unit_handle_t counter;
static rmt_channel_handle_t receiver;
static rmt_symbol_word_t *rx_buffer;
static timing_accumulator_t capture;
static unsigned capture_goal;
static atomic_bool capture_done;
static bool initialized;
static activity_reading_t last_activity;
static int64_t last_activity_time;
/**
 * @brief Timestamp index edges and enqueue rising edges for fresh RPM acquisitions.
 *
 * @param arg Unused task or ISR context.
 */
static void index_isr(void *arg)
{
    (void)arg;
    bool active=gpio_get_level(PIN_INDEX);
    int64_t now=esp_timer_get_time();
    portENTER_CRITICAL_ISR(&index_lock);
    index_state.active=active;
    if (active) {
        if (index_state.last_rise) {
            int64_t p=now-index_state.last_rise;
            index_state.period_us=p;
            if (!index_state.min_us || p<index_state.min_us) index_state.min_us=p;
            if (p>index_state.max_us) index_state.max_us=p;
        }
        index_state.last_rise=now; ++index_state.pulses;
    } else if (index_state.last_rise) index_state.width_us=now-index_state.last_rise;
    portEXIT_CRITICAL_ISR(&index_lock);
    BaseType_t wake=pdFALSE;
    if (active && xQueueSendFromISR(index_queue,&now,&wake)!=pdTRUE) atomic_store(&index_overflow,true);
    if (wake) portYIELD_FROM_ISR();
}
/**
 * @brief Reduce callback-owned RMT DMA segments in ISR context and publish completion.
 *
 * @param chan Unused RMT channel handle supplied by the driver.
 * @param e Callback-owned DMA segment view; consume before returning, never retain.
 * @param ctx Unused receiver callback context.
 * @return false because the worker polls completion instead of using task notifications.
 */
static bool rx_done(rmt_channel_handle_t chan, const rmt_rx_done_event_data_t *e, void *ctx)
{
    (void)chan; (void)ctx;
    if (atomic_load_explicit(&capture_done,memory_order_acquire)) return false;
    for (size_t i=0;i<e->num_symbols && capture.result.samples<capture_goal;++i) {
        rmt_symbol_word_t s=e->received_symbols[i];
        timing_accumulator_feed(&capture,s.level0,s.duration0,capture_goal);
        timing_accumulator_feed(&capture,s.level1,s.duration1,capture_goal);
    }
// Release/acquire publishes accumulated fields before the worker sees completion.
    if (capture.result.samples>=capture_goal || e->flags.is_last)
        atomic_store_explicit(&capture_done,true,memory_order_release);
    return false;
}
/**
 * @brief Allocate index, PCNT and RMT resources once; enable APIs only on full success.
 * @return ESP_OK on success, or the first prerequisite, sensor or peripheral error.
 */
esp_err_t measurements_init(void)
{
    index_queue=xQueueCreate(128,sizeof(int64_t));
    if (!index_queue) return ESP_ERR_NO_MEM;
    esp_err_t e=gpio_install_isr_service(0);
    if (e!=ESP_OK) return e;
    e=gpio_set_intr_type(PIN_INDEX,GPIO_INTR_ANYEDGE);
    if (e!=ESP_OK) return e;
    e=gpio_isr_handler_add(PIN_INDEX,index_isr,NULL);
    if (e!=ESP_OK) return e;
    pcnt_unit_config_t c={.low_limit=-30000,.high_limit=30000,.flags.accum_count=true};
    e=pcnt_new_unit(&c,&counter);
    if (e!=ESP_OK) return e;
    pcnt_chan_config_t ch={.edge_gpio_num=PIN_READ_DATA,.level_gpio_num=-1};
    pcnt_channel_handle_t channel;
    e=pcnt_new_channel(counter,&ch,&channel);
    if (e!=ESP_OK) return e;
    e=pcnt_channel_set_edge_action(channel,PCNT_CHANNEL_EDGE_ACTION_INCREASE,PCNT_CHANNEL_EDGE_ACTION_HOLD);
    if (e!=ESP_OK) return e;
    pcnt_glitch_filter_config_t filter={.max_glitch_ns=100};
    e=pcnt_unit_set_glitch_filter(counter,&filter);
    if (e!=ESP_OK) return e;
    e=pcnt_unit_add_watch_point(counter,30000);
    if (e!=ESP_OK) return e;
    e=pcnt_unit_add_watch_point(counter,-30000);
    if (e!=ESP_OK) return e;
    e=pcnt_unit_enable(counter);
    if (e!=ESP_OK) return e;
    rmt_rx_channel_config_t rc={.gpio_num=PIN_READ_DATA,.clk_src=RMT_CLK_SRC_DEFAULT,
        .resolution_hz=20000000,.mem_block_symbols=1024,.flags.with_dma=true};
    e=rmt_new_rx_channel(&rc,&receiver);
    if (e!=ESP_OK) return e;
    rmt_rx_event_callbacks_t callbacks={.on_recv_done=rx_done};
    e=rmt_rx_register_event_callbacks(receiver,&callbacks,NULL);
    if (e!=ESP_OK) return e;
    rx_buffer=heap_caps_aligned_calloc(64,1024,sizeof(*rx_buffer),MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    if (!rx_buffer) return ESP_ERR_NO_MEM;
    gpio_pullup_dis(PIN_READ_DATA); // PCNT config enables pull-up; buffered signal needs none
    initialized=true;
    return ESP_OK;
}
/**
 * @brief Copy index statistics under lock and recompute one-second freshness.
 *
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 */
void measurements_index(index_reading_t *out)
{
    portENTER_CRITICAL(&index_lock); *out=index_state; portEXIT_CRITICAL(&index_lock);
    out->active=floppy_hal_index();
    out->valid=out->period_us>0 && esp_timer_get_time()-out->last_rise<1000000;
}
/**
 * @brief Measure new complete index intervals with bounded cancellable waits.
 *
 * @param n Complete intervals requested; index allows 1–100, read timing allows 1–10000.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for complete acquisition; inspect passed separately for speed tolerance.
 */
esp_err_t measurements_rpm(unsigned n, rpm_reading_t *out)
{
    memset(out,0,sizeof(*out));
    if (!initialized) return ESP_ERR_INVALID_STATE;
    if (n<1 || n>100) return ESP_ERR_INVALID_ARG;
    if (!drive_state()->motor || drive_state()->selected<0) return ESP_ERR_INVALID_STATE;
// Ignore history: N complete intervals require N+1 new rising edges.
    xQueueReset(index_queue); atomic_store(&index_overflow,false);
    int64_t previous=0, total=0, min=INT64_MAX, max=0;
    for (unsigned i=0;i<=n;++i) {
        int64_t edge=0, deadline=esp_timer_get_time()+1000000;
        while (xQueueReceive(index_queue,&edge,0)!=pdTRUE) {
            esp_err_t e=drive_wait_ms(5);
            if (e!=ESP_OK) return e;
            if (esp_timer_get_time()>deadline) return ESP_ERR_TIMEOUT;
        }
        if (atomic_load(&index_overflow)) return ESP_ERR_INVALID_RESPONSE;
        if (previous) {
            int64_t p=edge-previous;
            if (p<=0) return ESP_ERR_INVALID_RESPONSE;
            if (p<min) min=p;
            if (p>max) max=p;
            total+=p;
        }
        previous=edge;
    }
    out->samples=n;
    out->rpm=60000000.0*n/total;
    out->rpm_min=60000000.0/max; out->rpm_max=60000000.0/min;
    out->deviation=100.0*fabs(out->rpm-300.0)/300.0;
    out->period_min_us=min; out->period_max_us=max; out->jitter_us=max-min;
    double tolerance=config_get()->rpm_tolerance;
    out->passed=out->deviation<=tolerance &&
        fabs(out->rpm_min-300.0)*100/300<=tolerance && fabs(out->rpm_max-300.0)*100/300<=tolerance;
    return ESP_OK;
}
/**
 * @brief Read the last successful activity result rather than a continuous live count.
 *
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @param age_us Non-NULL age destination in microseconds, or -1 before the first observation.
 * @return true when a previous successful observation exists.
 */
bool measurements_last_activity(activity_reading_t *out, int64_t *age_us)
{
    *out=last_activity;
    *age_us=last_activity_time?esp_timer_get_time()-last_activity_time:-1;
    return last_activity_time!=0;
}
/**
 * @brief Count read assertion edges with PCNT over a bounded observation window.
 *
 * @param ms Requested milliseconds; scheduler delays may extend elapsed time.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for a complete window, including zero pulses; caller classifies absence.
 */
esp_err_t measurements_activity(unsigned ms, activity_reading_t *out)
{
    memset(out,0,sizeof(*out));
    if (!initialized) return ESP_ERR_INVALID_STATE;
    if (ms<1 || ms>60000) return ESP_ERR_INVALID_ARG;
    if (!drive_state()->motor || drive_state()->selected<0) return ESP_ERR_INVALID_STATE;
    esp_err_t e=pcnt_unit_clear_count(counter);
    if (e!=ESP_OK) return e;
    int64_t start=esp_timer_get_time();
    e=pcnt_unit_start(counter);
    if (e!=ESP_OK) return e;
    e=drive_wait_ms(ms);
// Stop counting even when the cancellable observation wait failed.
    esp_err_t stop=pcnt_unit_stop(counter);
    out->duration_us=esp_timer_get_time()-start;
    if (e!=ESP_OK) return e;
    if (stop!=ESP_OK) return stop;
    int pulses;
    e=pcnt_unit_get_count(counter,&pulses);
    if (e!=ESP_OK) return e;
    if (pulses<0) return ESP_ERR_INVALID_RESPONSE;
    out->pulses=pulses;
    out->rate=pulses*1000000.0/out->duration_us;
    last_activity=*out; last_activity_time=esp_timer_get_time();
    return ESP_OK;
}
/**
 * @brief Capture a contiguous RMT frame and summarize complete rising-edge intervals.
 *
 * @param n Complete intervals requested; index allows 1–100, read timing allows 1–10000.
 * @param out Non-NULL result destination; partial acquisition data may accompany measurement errors.
 * @return ESP_OK for the requested count; partial data is returned on timeout.
 */
esp_err_t measurements_timing(unsigned n, timing_reading_t *out)
{
    memset(out,0,sizeof(*out));
    if (!initialized) return ESP_ERR_INVALID_STATE;
    if (n<1 || n>10000) return ESP_ERR_INVALID_ARG;
    if (!drive_state()->motor || drive_state()->selected<0) return ESP_ERR_INVALID_STATE;
    timing_accumulator_init(&capture); capture_goal=n;
    atomic_store(&capture_done,false);
    esp_err_t e=rmt_enable(receiver);
    if (e!=ESP_OK) return e;
    rmt_receive_config_t c={.signal_range_min_ns=100,.signal_range_max_ns=1000000,
        .flags.en_partial_rx=true};
    e=rmt_receive(receiver,rx_buffer,1024*sizeof(*rx_buffer),&c);
    int64_t deadline=esp_timer_get_time()+2000000;
    while (e==ESP_OK && !atomic_load_explicit(&capture_done,memory_order_acquire)) {
        e=drive_wait_ms(1);
        if (e==ESP_OK && esp_timer_get_time()>deadline) e=ESP_ERR_TIMEOUT;
    }
    esp_err_t stop=rmt_disable(receiver); // quiesces DMA/ISR before reusing accumulator
    if (e==ESP_OK) e=stop;
    *out=capture.result;
    if (out->samples==0) out->min_ticks=0;
    if (e==ESP_OK && out->samples<n) e=ESP_ERR_TIMEOUT; // no invented intervals across gaps
    return e;
}
