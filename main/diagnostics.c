#include "diagnostics.h"
#include <stdio.h>
#include "drive.h"
#include "config.h"
#include "floppy_hal.h"
#include "measurements.h"
#include "power_monitor.h"
static esp_err_t prepare(void)
{
    drive_stop();
    if (!power_monitor_healthy()) { puts("FAIL: rail health / INA226 unavailable"); return ESP_ERR_INVALID_STATE; }
    esp_err_t e=drive_select(config_get()->drive_select);
    if (e==ESP_OK) e=home();
    printf("Home: %s\n",e==ESP_OK?"PASS":"FAIL");
    return e;
}
static esp_err_t finish(esp_err_t e)
{
    bool cancelled=drive_cancelled();
    drive_stop(); // tests never leave motor or selection asserted
    printf("RESULT: %s%s\n",e==ESP_OK?"PASS":"FAIL",cancelled?" (cancelled)":"");
    return e;
}
esp_err_t diagnostics_basic(void)
{
    puts("BASIC: formatted disk required; physical Write Enable OFF");
    esp_err_t e=prepare();
    if (e!=ESP_OK) return finish(e);
    puts("Controller / rail health: PASS; drive select: commanded (no electrical feedback)");
    // Prove that Track0 releases, then returns; a stuck-high input cannot pass this test.
    e=step_in();
    if (e==ESP_OK) e=seek(0);
    if (e==ESP_OK) e=motor_on();
    if (e!=ESP_OK) { puts("FAIL: Track0 release/return or motor start"); return finish(e); }
    rpm_reading_t rpm;
    e=measurements_rpm(config_get()->rpm_samples,&rpm);
    if (e!=ESP_OK) { puts("FAIL: Index/RPM acquisition"); return finish(e); }
    printf("Index/RPM: %.3f RPM %s\n",rpm.rpm,rpm.passed?"PASS":"FAIL");
    if (!rpm.passed) return finish(ESP_ERR_INVALID_RESPONSE);
    printf("Write Protect: %s (observed; toggle sensor manually to validate)\n",floppy_hal_wp()?"ACTIVE":"INACTIVE");
    activity_reading_t activity;
    e=measurements_activity(config_get()->read_sample_ms,&activity);
    if (e==ESP_OK && !activity.pulses) e=ESP_ERR_NOT_FOUND;
    printf("Read activity: %lu pulses %s\n",(unsigned long)activity.pulses,e==ESP_OK?"PASS":"FAIL");
    return finish(e);
}
esp_err_t diagnostics_seek(void)
{
    esp_err_t e=prepare();
    if (e!=ESP_OK) return finish(e);
    unsigned targets[]={10,20,config_get()->max_track,0};
    for (unsigned i=0;i<4;++i) {
        if (targets[i]>config_get()->max_track) continue;
        e=seek(targets[i]);
        printf("Seek %u: %s\n",targets[i],e==ESP_OK?"PASS":"FAIL");
        if (e!=ESP_OK) break;
    }
    // Do not re-home here: seek(0) must itself agree with the sensor, not mask lost steps.
    return finish(e);
}
esp_err_t diagnostics_exercise(unsigned cycles)
{
    if (!cycles || cycles>100000) return ESP_ERR_INVALID_ARG;
    esp_err_t e=prepare();
    if (e!=ESP_OK) return finish(e);
    unsigned completed=0;
    for (;completed<cycles;++completed) {
        e=seek(config_get()->max_track);
        if (e!=ESP_OK) { puts("FAIL: outward-to-higher-track leg"); break; }
        e=seek(0);
        if (e!=ESP_OK) { puts("FAIL: return-to-Track0 leg"); break; }
        if ((completed+1)%10==0 || completed+1==cycles) printf("Cycle %u/%u\n",completed+1,cycles);
    }
    printf("Cycles completed: %u/%u\nSeek/Track0 errors: %u\n",completed,cycles,
        e!=ESP_OK && !drive_cancelled()?1u:0u);
    return finish(e);
}
