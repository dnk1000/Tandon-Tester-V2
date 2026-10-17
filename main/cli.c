#include "cli.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdatomic.h>
#include <inttypes.h>
#include "command.h"
#include "drive.h"
#include "floppy_hal.h"
#include "measurements.h"
#include "power_monitor.h"
#include "diagnostics.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_system.h"
#include "esp_app_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
static QueueHandle_t commands;
static atomic_bool busy;
static void help(const char *topic)
{
    const struct { const char *name, *usage; } entries[]={
        {"help","help [command]"}, {"version","version"}, {"status","status"},
        {"reset","reset (reboot with all controls deasserted)"},
        {"select","select <0|1|2|3|off>"}, {"motor","motor <on|off>"},
        {"home","home (auto-selects configured drive; bounded restore)"},
        {"seek","seek <track> (home first)"}, {"step","step <in|out> [count] (home first)"},
        {"track0","track0"}, {"wp","wp"}, {"index","index"}, {"rpm","rpm [1..100 samples]"},
        {"read","read activity [0.001..60 seconds]\nread timing [1..10000 pulse intervals]"},
        {"power","power\npower watch (Ctrl-C or stop to end)"},
        {"test","test basic\ntest seek\ntest exercise <1..100000 cycles>"},
        {"config","config show|save|defaults\nconfig step_time <5..1000 ms>\nconfig max_track <1..79>\n"
            "config home_limit <max_track+1..200>\nconfig rpm_samples <1..100>\n"
            "config rpm_tolerance <0.1..20 percent>\nconfig drive_select <0..3>\n"
            "config motor_spinup_ms <300..10000>\nconfig read_sample_ms <1..60000>"}};
    for (unsigned i=0;i<sizeof(entries)/sizeof(entries[0]);++i)
        if (!*topic || strcmp(topic,entries[i].name)==0) puts(entries[i].usage);
    puts("No write commands. Ctrl-C / stop deasserts controls and invalidates position.");
}
static void power_print(void)
{
    power_reading_t p;
    power_monitor_snapshot(&p);
    const rail_reading_t *r[]={&p.rail5,&p.rail12};
    const char *names[]={"+5V","+12V"};
    for (unsigned i=0;i<2;++i) {
        if (!r[i]->valid) printf("%s: UNAVAILABLE (%s)\n",names[i],esp_err_to_name(r[i]->error));
        else printf("%s: %.3f V %.4f A UV=%d OV=%d OC=%d ALERT=%d OVF=%d\n",names[i],
            r[i]->voltage,r[i]->current,r[i]->under_voltage,r[i]->over_voltage,
            r[i]->over_current,r[i]->alert,r[i]->overflow);
    }
    if (p.rail5.valid && p.rail12.valid)
        printf("Total: %.3f W\n",p.rail5.voltage*p.rail5.current+p.rail12.voltage*p.rail12.current);
    printf("Power healthy: %s; fault input: %d; sample age: %" PRId64 " ms\n",
        power_monitor_healthy()?"YES":"NO",floppy_hal_power_fault(),
        p.timestamp?(floppy_hal_now_us()-p.timestamp)/1000:-1);
}
static void index_print(void)
{
    index_reading_t i;
    measurements_index(&i);
    printf("INDEX: %s valid=%d pulses=%" PRIu32 "\n",i.active?"ACTIVE":"INACTIVE",i.valid,i.pulses);
    if (i.valid) printf("Period: %.3f ms width: %.3f ms frequency: %.4f Hz RPM: %.3f\n"
        "Period min/max: %.3f/%.3f ms jitter: %.3f ms (since boot)\n",
        i.period_us/1000.0,i.width_us/1000.0,1000000.0/i.period_us,60000000.0/i.period_us,
        i.min_us/1000.0,i.max_us/1000.0,(i.max_us-i.min_us)/1000.0);
}
static void config_print(void)
{
    const tester_config_t *c=config_get();
    printf("step_time=%" PRIu32 " ms\nmax_track=%" PRIu32 "\nhome_limit=%" PRIu32
        "\nrpm_samples=%" PRIu32 "\nrpm_tolerance=%.3f %%\ndrive_select=%" PRIu32
        "\nmotor_spinup_ms=%" PRIu32 "\nread_sample_ms=%" PRIu32 "\n",
        c->step_time,c->max_track,c->home_limit,c->rpm_samples,(double)c->rpm_tolerance,
        c->drive_select,c->motor_spinup_ms,c->read_sample_ms);
}
static esp_err_t execute(const command_t *c)
{
    switch (c->kind) {
    case CMD_HELP: help(c->name); break;
    case CMD_VERSION: printf("TM100 Revision A %s; ESP-IDF %s; writes disabled\n",esp_app_get_description()->version,esp_get_idf_version()); break;
    case CMD_STATUS: {
        const drive_state_t *s=drive_state();
        printf("Selected: %d Motor: %s Direction: %s Track: %d Valid: %d\n",
            s->selected,s->motor?"ON":"OFF",s->inward?"IN":"OUT",s->track,s->position_valid);
        printf("Track0: %d Write Protect: %d HW Write Enable: %d Write armed: NO\n",
            floppy_hal_track0(),floppy_hal_wp(),floppy_hal_hw_write_enabled());
        activity_reading_t activity; int64_t age;
        if (measurements_last_activity(&activity,&age))
            printf("Last read activity: %" PRIu32 " pulses %.3f /s (age %" PRId64 " ms)\n",activity.pulses,activity.rate,age/1000);
        else puts("Read activity: not yet measured");
        index_print(); power_print(); break;
    }
    case CMD_RESET: drive_stop(); puts("OK: resetting"); fflush(stdout); usb_serial_jtag_wait_tx_done(pdMS_TO_TICKS(50)); esp_restart(); break;
    case CMD_SELECT: if (c->value<0) drive_deselect(); else return drive_select(c->value); break;
    case CMD_MOTOR: if (c->flag) return motor_on(); motor_off(); break;
    case CMD_HOME: return home();
    case CMD_SEEK: return seek(c->value);
    case CMD_STEP: {
        const drive_state_t *s=drive_state();
        if (!s->position_valid) return ESP_ERR_INVALID_STATE;
        // Preflight entire request so an out-of-bounds count never partially moves.
        int destination=s->track+(c->flag?c->value:-c->value);
        if (destination<0 || destination>(int)config_get()->max_track) return ESP_ERR_INVALID_ARG;
        return seek(destination);
    }
    case CMD_TRACK0: printf("TRACK0: %s\n",floppy_hal_track0()?"ACTIVE":"INACTIVE"); break;
    case CMD_WP: printf("WRITE PROTECT: %s\n",floppy_hal_wp()?"ACTIVE":"INACTIVE"); break;
    case CMD_INDEX: index_print(); break;
    case CMD_RPM: {
        rpm_reading_t r;
        esp_err_t e=measurements_rpm(c->value,&r);
        if (e!=ESP_OK) return e;
        printf("Samples: %u RPM avg/min/max: %.3f/%.3f/%.3f Deviation: %.3f %%\n"
            "Period min/max: %.3f/%.3f us jitter: %.3f us\nRESULT: %s\n",
            r.samples,r.rpm,r.rpm_min,r.rpm_max,r.deviation,r.period_min_us,r.period_max_us,r.jitter_us,r.passed?"PASS":"FAIL");
        return r.passed?ESP_OK:ESP_ERR_INVALID_RESPONSE;
    }
    case CMD_READ_ACTIVITY: {
        activity_reading_t r;
        esp_err_t e=measurements_activity(c->value,&r);
        if (e!=ESP_OK) return e;
        printf("Read asserted edges: %" PRIu32 " Duration: %.6f s Rate: %.3f /s Present: %s\n",
            r.pulses,r.duration_us/1000000.0,r.rate,r.pulses?"YES":"NO");
        return r.pulses?ESP_OK:ESP_ERR_NOT_FOUND;
    }
    case CMD_READ_TIMING: {
        timing_reading_t r;
        esp_err_t e=measurements_timing(c->value,&r);
        printf("Captured pulse intervals: %" PRIu32 "/%" PRId32 " (50 ns ticks)\n",r.samples,c->value);
        if (r.samples) {
            printf("Min/max/mean: %.3f/%.3f/%.3f us\n",r.min_ticks/20.0,r.max_ticks/20.0,r.sum_ticks/(20.0*r.samples));
            for (unsigned i=0;i<16;++i) printf("%u%s us: %" PRIu32 "\n",i,i==15?"+":"..next",r.histogram[i]);
        }
        return e;
    }
    case CMD_POWER: power_print(); break;
    case CMD_POWER_WATCH:
        while (!drive_cancelled()) {
            power_print();
            // Monitoring remains useful with bad/unpowered rails; motion is stopped on each poll.
            for (unsigned i=0;i<100 && !drive_cancelled();++i) {
                drive_poll(); vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
        drive_stop(); return ESP_ERR_INVALID_STATE;
    case CMD_TEST_BASIC: return diagnostics_basic();
    case CMD_TEST_SEEK: return diagnostics_seek();
    case CMD_TEST_EXERCISE: return diagnostics_exercise(c->value);
    case CMD_CONFIG_SHOW: config_print(); break;
    case CMD_CONFIG_SET: drive_stop(); return config_apply(&c->config);
    case CMD_CONFIG_SAVE: return config_save();
    case CMD_CONFIG_DEFAULTS: {
        tester_config_t defaults; config_defaults(&defaults); drive_stop(); return config_apply(&defaults);
    }
    }
    return ESP_OK;
}
static void worker(void *arg)
{
    (void)arg;
    char line[CLI_LINE_SIZE];
    for (;;) {
        if (xQueueReceive(commands,line,pdMS_TO_TICKS(10))!=pdTRUE) { drive_poll(); continue; }
        drive_poll(); // consume a pending stop even if a later command cleared cancellation
        command_t c;
        if (drive_cancelled() || !usb_serial_jtag_is_connected()) {
            drive_stop(); puts("ERROR: interrupted or USB disconnected");
        } else if (!command_parse(line,config_get(),&c)) {
            drive_stop(); puts("ERROR: invalid command/value; controls deasserted. Use help.");
        } else {
            esp_err_t e=execute(&c);
            if (e!=ESP_OK) {
                drive_stop();
                printf("ERROR: %s%s; controls deasserted. Home required before seek.\n",esp_err_to_name(e),
                    drive_cancelled()?" (cancelled)":"");
            } else puts("OK");
        }
        atomic_store(&busy,false);
        printf("tm100> "); fflush(stdout);
    }
}
static void input_task(void *arg)
{
    (void)arg;
    char line[CLI_LINE_SIZE]={0};
    size_t used=0;
    bool discard=false, was_connected=false, last_cr=false;
    puts("TM100 Revision A. Writes disabled. Type help. Ctrl-C / stop aborts.");
    printf("tm100> "); fflush(stdout);
    for (;;) {
        bool connected=usb_serial_jtag_is_connected();
        if (was_connected && !connected) { drive_cancel(); used=0; discard=false; }
        was_connected=connected;
        unsigned char ch;
        int count=usb_serial_jtag_read_bytes(&ch,1,pdMS_TO_TICKS(10));
        if (count!=1) continue;
        if (ch==3) {
            drive_cancel(); used=0; discard=false;
            puts("\nCancelled; controls releasing."); last_cr=false; continue;
        }
        if (ch=='\n' && last_cr) { last_cr=false; continue; }
        last_cr=ch=='\r';
        if (ch=='\r' || ch=='\n') {
            putchar('\n');
            line[used]=0;
            if (discard) { drive_cancel(); puts("ERROR: line too long or invalid bytes; discarded"); }
            else if (used) {
                if (strcasecmp(line,"stop")==0) {
                    drive_cancel(); puts("Stopped; controls releasing.");
                } else if (atomic_exchange(&busy,true)) {
                    // Reject queued motion; operator must wait for prompt. Invalid input is fail-safe.
                    drive_cancel(); puts("ERROR: busy; command rejected and operation cancelled");
                } else {
                    drive_clear_cancel();
                    if (xQueueSend(commands,line,0)!=pdTRUE) { drive_cancel(); atomic_store(&busy,false); }
                }
            } else if (!atomic_load(&busy)) { printf("tm100> "); fflush(stdout); }
            used=0; discard=false; continue;
        }
        if (ch==8 || ch==127) { if (used && !discard) { --used; printf("\b \b"); } continue; }
        if (ch=='\t' || (ch>=32 && ch<=126)) {
            if (!discard && used<sizeof(line)-1) { line[used++]=ch; putchar(ch); }
            else { discard=true; drive_cancel(); }
        } else { discard=true; drive_cancel(); }
    }
}
esp_err_t cli_start(void)
{
    usb_serial_jtag_driver_config_t c={.tx_buffer_size=2048,.rx_buffer_size=1024};
    esp_err_t e=usb_serial_jtag_driver_install(&c);
    if (e!=ESP_OK) return e;
    usb_serial_jtag_vfs_use_driver();
    setvbuf(stdout,NULL,_IONBF,0);
    commands=xQueueCreate(1,CLI_LINE_SIZE);
    if (!commands) return ESP_ERR_NO_MEM;
    if (xTaskCreate(worker,"commands",8192,NULL,4,NULL)!=pdPASS) return ESP_ERR_NO_MEM;
    if (xTaskCreate(input_task,"usb_input",4096,NULL,6,NULL)!=pdPASS) return ESP_ERR_NO_MEM;
    return ESP_OK;
}
