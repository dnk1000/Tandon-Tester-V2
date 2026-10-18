/**
 * @file test_main.c
 * @brief Host-only test main fixture or declaration; not a physical hardware validation.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "command.h"
#include "drive.h"
#include "floppy_hal.h"
#include "power_monitor.h"
#include "measurements.h"
#include "diagnostics.h"
static tester_config_t cfg;
static int physical, selected, pulses;
static bool motor, inward, healthy, stuck_high, stuck_low;
static bool rpm_pass, read_present;
static int64_t clock_us, previous_pulse, cancel_at, fault_at;
static unsigned checks;
/**
 * @brief Expose the fixture configuration to production drive and diagnostic code.
 */
const tester_config_t *config_get(void) { return &cfg; }
/**
 * @brief Require fresh valid rails and a deasserted live hardware fault input.
 */
bool power_monitor_healthy(void) { return healthy; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
int64_t floppy_hal_now_us(void) { return clock_us; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_safe(void) { selected=-1; motor=false; inward=false; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_select(int ds) { selected=ds; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_motor(bool on) { motor=on; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_direction(bool in) { inward=in; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
bool floppy_hal_track0(void) { return stuck_high || (!stuck_low && physical==0); }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
bool floppy_hal_wp(void) { return true; }
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_delay_us(uint32_t us)
{
    clock_us+=us;
    if (cancel_at && clock_us>=cancel_at) drive_cancel();
    if (fault_at && clock_us>=fault_at) healthy=false;
}
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
void floppy_hal_step_pulse(void)
{
    assert(selected>=0 && selected<4);
    if (previous_pulse) assert(clock_us-previous_pulse>=5000);
    previous_pulse=clock_us;
    ++pulses;
    if (inward) ++physical;
    else if (physical>0) --physical;
    clock_us+=25;
}
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
esp_err_t measurements_rpm(unsigned n, rpm_reading_t *out)
{
    assert(n==cfg.rpm_samples); assert(motor && selected>=0);
    *out=(rpm_reading_t){.rpm=300,.passed=rpm_pass}; return ESP_OK;
}
/**
 * @brief Supply or record fixture hardware state for the production code under test.
 */
esp_err_t measurements_activity(unsigned ms, activity_reading_t *out)
{
    assert(ms==cfg.read_sample_ms); assert(motor && selected>=0);
    *out=(activity_reading_t){.pulses=read_present?123:0}; return ESP_OK;
}
/**
 * @brief Reset the mechanism fixture and start production drive code at a chosen physical track.
 */
static void fresh(int position)
{
    config_defaults(&cfg); healthy=true; stuck_high=stuck_low=false;
    rpm_pass=read_present=true;
    physical=position; pulses=0; previous_pulse=0; clock_us+=1000000;
    cancel_at=fault_at=0; drive_clear_cancel(); drive_init();
}
/**
 * @brief Assert that a completed stop leaves motor/select released and position unknown.
 */
static void safe(void)
{
    assert(selected==-1 && !motor && !drive_state()->position_valid && drive_state()->track==-1);
}
/**
 * @brief Test defaults, cross-field bounds and non-finite configuration rejection.
 */
static void test_defaults(void)
{
    fresh(5); assert(config_validate(&cfg)); assert(cfg.rpm_tolerance==1.5f);
    tester_config_t bad=cfg; bad.step_time=4; assert(!config_validate(&bad));
    bad=cfg; bad.home_limit=39; assert(!config_validate(&bad));
    bad=cfg; bad.rpm_tolerance=NAN; assert(!config_validate(&bad));
    bad=cfg; bad.rpm_tolerance=INFINITY; assert(!config_validate(&bad));
    bad=cfg; bad.motor_spinup_ms=299; assert(!config_validate(&bad));
    bad=cfg; bad.max_track=80; assert(!config_validate(&bad));
    bad=cfg; bad.drive_select=4; assert(!config_validate(&bad));
    bad=cfg; bad.read_sample_ms=0; assert(!config_validate(&bad));
    checks++;
}
/**
 * @brief Test motion limits, homing, sensor failures, cancellation ordering and power faults.
 */
static void test_drive(void)
{
    fresh(15); safe(); assert(seek(0)!=ESP_OK); assert(step_in()!=ESP_OK); assert(!pulses); checks++;
    assert(home()==ESP_OK); assert(selected==0 && pulses==15 && physical==0);
    assert(drive_state()->position_valid && drive_state()->track==0); checks++;
    assert(seek(39)==ESP_OK && physical==39 && drive_state()->track==39);
    int before=pulses; assert(step_in()!=ESP_OK && seek(40)!=ESP_OK && pulses==before); checks++;
    assert(seek(0)==ESP_OK && physical==0); before=pulses;
    assert(step_out()!=ESP_OK && pulses==before); checks++;
    assert(drive_select(1)==ESP_OK); assert(!drive_state()->position_valid); assert(seek(10)!=ESP_OK); checks++;
    assert(home()==ESP_OK); assert(motor_on()==ESP_OK); drive_deselect(); safe(); checks++;
    fresh(0); assert(motor_on()==ESP_OK); assert(motor && selected==-1); motor_off(); assert(!motor); checks++;
    fresh(50); assert(home()==ESP_OK && pulses==50); checks++;
    fresh(51); assert(home()==ESP_ERR_TIMEOUT && pulses==50); safe(); checks++;
    fresh(0); stuck_low=true; assert(home()==ESP_ERR_TIMEOUT && pulses==50); safe(); checks++;
    fresh(0); stuck_high=true; assert(home()==ESP_OK); assert(step_in()==ESP_ERR_INVALID_RESPONSE); safe(); checks++;
    fresh(2); assert(home()==ESP_OK); assert(seek(5)==ESP_OK); physical=0; drive_poll(); safe(); checks++;
    fresh(5); cancel_at=clock_us+4000; assert(home()!=ESP_OK); safe(); assert(pulses<=1); checks++;
    fresh(5); fault_at=clock_us+4000; assert(home()!=ESP_OK); safe(); checks++;
    fresh(0); healthy=false; assert(drive_select(0)!=ESP_OK && motor_on()!=ESP_OK); safe(); checks++;
    fresh(0); assert(home()==ESP_OK); assert(motor_on()==ESP_OK); healthy=false; drive_poll(); safe(); checks++;
    fresh(0); assert(home()==ESP_OK); cfg.step_time=0; assert(step_in()==ESP_OK); assert(step_in()==ESP_OK); checks++;
    fresh(0); assert(home()==ESP_OK); assert(motor_on()==ESP_OK);
    drive_cancel(); drive_clear_cancel(); drive_poll(); safe(); checks++;
    fresh(0); assert(home()==ESP_OK); assert(drive_select(4)==ESP_ERR_INVALID_ARG && selected==0); checks++;
}
/**
 * @brief Test accepted command forms and malformed, overflowing or destructive input rejection.
 */
static void test_parser(void)
{
    fresh(0); command_t c;
    const char *valid[]={"help", "HELP SeEk", "version", "status", "reset", "select 0", "select 3", "select off",
        "motor on","motor off","home","seek 39","step in","step out 2","track0","wp","index","rpm","rpm 100",
        "read activity","read activity 0.125","read timing","read timing 10000","power","power watch",
        "test basic","test seek","test exercise 100","config show","config save","config defaults",
        "config step_time 5","config max_track 40","config home_limit 100","config rpm_samples 20",
        "config rpm_tolerance 1.5","config drive_select 3","config motor_spinup_ms 300","config read_sample_ms 500"};
    for (unsigned i=0;i<sizeof(valid)/sizeof(valid[0]);++i) {
        if (!command_parse(valid[i],&cfg,&c)) fprintf(stderr,"Unexpected rejection: %s\n",valid[i]);
        assert(command_parse(valid[i],&cfg,&c));
    }
    const char *invalid[]={"", " ", "writegate on", "write arm", "select -1","select 4","select 0 garbage",
        "motor on 10","seek -1","seek 40","seek 1.5","seek +1","seek 99999999999999999999",
        "step in 0","step in 40","step sideways","step out -1","rpm 0","rpm 101","rpm nan",
        "read activity 0","read activity nan","read activity 1e2","read activity 1..2","read timing 10001",
        "test exercise 0","test exercise -1","test exercise 100001","config step_time 4",
        "config max_track 50","config home_limit 39","config rpm_tolerance nan","config rpm_tolerance inf",
        "config rpm_tolerance 0","config rpm_tolerance 21","config drive_select 4","help write","help seek extra",
        "config save extra", "a b c d e f", "config motor_spinup_ms 299"};
    for (unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) assert(!command_parse(invalid[i],&cfg,&c));
    assert(command_parse("config rpm_tolerance 1.5",&cfg,&c) && c.config.rpm_tolerance==1.5f);
    assert(command_parse("read activity 0.125",&cfg,&c) && c.value==125);
    char longline[200]; memset(longline,'a',199);longline[199]=0;assert(!command_parse(longline,&cfg,&c));
    safe(); assert(pulses==0); checks++;
}
/**
 * @brief Test interval statistics, initial fragments and DMA block continuity.
 */
static void test_timing(void)
{
    timing_accumulator_t a; timing_accumulator_init(&a);
    // Arbitrary initial HIGH fragment is not a complete assertion edge.
    timing_accumulator_feed(&a,1,5,3); timing_accumulator_feed(&a,0,20,3);
    timing_accumulator_feed(&a,1,20,3); timing_accumulator_feed(&a,0,20,3);
    timing_accumulator_feed(&a,1,20,3); timing_accumulator_feed(&a,0,60,3);
    timing_accumulator_feed(&a,1,20,3); timing_accumulator_feed(&a,0,100,3);
    timing_accumulator_feed(&a,1,20,3);
    assert(a.result.samples==3 && a.result.min_ticks==40 && a.result.max_ticks==120);
    assert(a.result.sum_ticks==240 && a.result.histogram[2]==1 && a.result.histogram[4]==1 && a.result.histogram[6]==1);
    timing_accumulator_feed(&a,0,20,3); timing_accumulator_feed(&a,1,20,3); assert(a.result.samples==3); checks++;
    timing_accumulator_init(&a);
    timing_accumulator_feed(&a,0,20,2); timing_accumulator_feed(&a,1,10,2);
    timing_accumulator_feed(&a,1,10,2); // same level across DMA blocks isn't a second rising edge
    timing_accumulator_feed(&a,0,20,2); timing_accumulator_feed(&a,1,20,2);
    assert(a.result.samples==1 && a.result.min_ticks==40); checks++;
}
/**
 * @brief Test signed INA226 scaling and rail fault classification.
 */
static void test_power(void)
{
    assert(fabsf(ina226_bus_voltage(4000)-5)<0.00001f);
    assert(fabsf(ina226_bus_voltage(9600)-12)<0.00001f);
    assert(fabsf(ina226_shunt_current(6000)-0.6f)<0.00001f);
    assert(fabsf(ina226_shunt_current((uint16_t)(int16_t)-6000)+0.6f)<0.00001f);
    rail_reading_t r={.voltage=5,.current=.6,.valid=true}; assert(!power_rail_fault(&r,5));
    r.voltage=4.7; assert(power_rail_fault(&r,5) && r.under_voltage);
    r.voltage=5.3; assert(power_rail_fault(&r,5) && r.over_voltage);
    r.voltage=5; r.current=2.1; assert(power_rail_fault(&r,5) && r.over_current);
    r.current=-.1; assert(power_rail_fault(&r,5));
    r=(rail_reading_t){0}; assert(power_rail_fault(&r,5)); checks++;
}
/**
 * @brief Run production diagnostics against controlled mechanism and measurement doubles.
 */
static void test_diagnostics(void)
{
    fresh(10); assert(diagnostics_basic()==ESP_OK);safe();checks++;
    fresh(10); assert(diagnostics_seek()==ESP_OK);safe();checks++;
    fresh(10); assert(diagnostics_exercise(100)==ESP_OK);assert(pulses==7810);safe();checks++;
    fresh(0); rpm_pass=false; assert(diagnostics_basic()!=ESP_OK);safe();checks++;
    fresh(0); read_present=false; assert(diagnostics_basic()!=ESP_OK);safe();checks++;
    fresh(10); cancel_at=clock_us+200000; assert(diagnostics_exercise(100)!=ESP_OK);safe();checks++;
    fresh(10); stuck_high=true; assert(diagnostics_seek()!=ESP_OK);safe();checks++;
}
/**
 * @brief Run the main host fixture assertions.
 */
int main(void)
{
    test_defaults();test_drive();test_parser();test_timing();test_power();test_diagnostics();
    printf("PASS: %u safety/functional test groups\n",checks);
    return 0;
}
