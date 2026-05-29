#include "timer.h"

// ARM Generic Timer (for uptime ticks)
static inline unsigned long long read_cntpct(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

// PL031 RTC registers (for real date/time)
#define RTC_BASE ((volatile unsigned int*)0x09010000)
#define RTC_DR   0x00  // Data register

static unsigned long long last_ticks = 0;
static unsigned int seconds_since_boot = 0;
static unsigned int rtc_initialized = 0;

void timer_init(void) {
    last_ticks = read_cntpct();
    seconds_since_boot = 0;
    
    // Check if RTC exists
    unsigned int rtc_value = RTC_BASE[RTC_DR / 4];
    if (rtc_value != 0) {
        rtc_initialized = 1;
    }
}

void timer_update(void) {
    unsigned long long now = read_cntpct();
    if (last_ticks == 0) {
        last_ticks = now;
        return;
    }
    unsigned long long diff = now - last_ticks;
    unsigned long long freq = 62500000;  // QEMU frequency
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

unsigned int rtc_get_seconds_since_epoch(void) {
    if (rtc_initialized) {
        return RTC_BASE[RTC_DR / 4];
    }
    return 0;
}

void rtc_get_datetime(unsigned int *year, unsigned int *month, unsigned int *day,
                       unsigned int *hour, unsigned int *min, unsigned int *sec) {
    unsigned int epoch = rtc_get_seconds_since_epoch();
    
    if (epoch == 0) {
        // Fallback to uptime-based time if RTC not available
        *hour = timer_get_hour();
        *min = 0;
        *sec = 0;
        *day = 1;
        *month = 1;
        *year = 2024;
        return;
    }
    
    // Convert epoch to date/time (approximate)
    unsigned int days = epoch / 86400;
    unsigned int seconds_remaining = epoch % 86400;
    
    *hour = seconds_remaining / 3600;
    *min = (seconds_remaining % 3600) / 60;
    *sec = seconds_remaining % 60;
    
    // Approximate date (starting from 1970-01-01)
    *year = 1970 + (days / 365);
    *month = ((days % 365) / 30) + 1;
    if (*month > 12) *month = 12;
    *day = ((days % 365) % 30) + 1;
    if (*day > 28) *day = 28;
}
