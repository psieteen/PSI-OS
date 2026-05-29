#ifndef TIMER_H
#define TIMER_H

void timer_init(void);
void timer_update(void);
unsigned int timer_get_seconds(void);
unsigned int timer_get_hour(void);

// Real date/time functions
unsigned int rtc_get_seconds_since_epoch(void);
void rtc_get_datetime(unsigned int *year, unsigned int *month, unsigned int *day,
                       unsigned int *hour, unsigned int *min, unsigned int *sec);

#endif
