/**
 * @file config_values.c
 * @brief Portable defaults and configuration bounds.
 */
#include "config.h"
#include <math.h>
/**
 * @brief Populate conservative TM-100-1 Revision A settings.
 *
 * @param c Non-NULL configuration candidate or destination.
 */
void config_defaults(tester_config_t *c)
{
    *c = (tester_config_t){.step_time=6, .max_track=39, .home_limit=50,
        .rpm_samples=10, .rpm_tolerance=1.5f, .drive_select=0, .motor_spinup_ms=500, .read_sample_ms=1000};
}
/**
 * @brief Validate every field and cross-field limits before accepting settings.
 *
 * @param c Candidate settings; NULL is rejected.
 * @return true only when every setting and cross-field relationship is valid.
 */
bool config_validate(const tester_config_t *c)
{
    return c && c->step_time >= 5 && c->step_time <= 1000 &&
        c->max_track >= 1 && c->max_track <= 79 &&
        c->home_limit > c->max_track && c->home_limit <= 200 &&
        c->rpm_samples >= 1 && c->rpm_samples <= 100 &&
        isfinite(c->rpm_tolerance) && c->rpm_tolerance >= 0.1f && c->rpm_tolerance <= 20 &&
        c->drive_select <= 3 && c->motor_spinup_ms >= 300 && c->motor_spinup_ms <= 10000 &&
        c->read_sample_ms >= 1 && c->read_sample_ms <= 60000;
}
