#include "measurements.h"
#include <string.h>
#include <limits.h>
void timing_accumulator_init(timing_accumulator_t *a)
{
    memset(a,0,sizeof(*a)); a->previous_level=-1; a->result.min_ticks=UINT32_MAX;
}
void timing_accumulator_feed(timing_accumulator_t *a, unsigned level, uint32_t ticks, unsigned limit)
{
    if (!ticks || a->result.samples>=limit) return;
    // Ignore the initial fragment; only transitions from a known LOW start pulses.
    if (level && a->previous_level==0) {
        if (a->have_rise) {
            uint32_t interval=(uint32_t)(a->elapsed-a->last_rise);
            timing_reading_t *r=&a->result;
            if (interval<r->min_ticks) r->min_ticks=interval;
            if (interval>r->max_ticks) r->max_ticks=interval;
            r->sum_ticks+=interval;
            ++r->samples;
            unsigned bin=interval/20; // 20MHz: 1us bins [n,n+1), last >=15us
            ++r->histogram[bin<15?bin:15];
        }
        a->last_rise=a->elapsed;
        a->have_rise=true;
    }
    a->previous_level=(int)level;
    a->elapsed+=ticks;
}
