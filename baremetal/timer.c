#include "timer.h"

static inline unsigned long long read_cntpct(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

static unsigned long long last_ticks = 0;
static unsigned int seconds_since_boot = 0;

void timer_init(void) {
    last_ticks = read_cntpct();
    seconds_since_boot = 0;
}

void timer_update(void) {
    unsigned long long now = read_cntpct();
    if (last_ticks == 0) {
        last_ticks = now;
        return;
    }
    unsigned long long diff = now - last_ticks;
    unsigned long long freq = 62500000;
    unsigned int seconds_passed = diff / freq;
    if (seconds_passed > 0) {
        seconds_since_boot += seconds_passed;
        last_ticks = now;
    }
}

unsigned int timer_get_seconds(void) {
    return seconds_since_boot;
}

unsigned int timer_get_hour(void) {
    return (seconds_since_boot / 3600) % 24;
}