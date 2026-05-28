#ifndef TIMER_H
#define TIMER_H

void timer_init(void);
void timer_update(void);
unsigned int timer_get_seconds(void);
unsigned int timer_get_hour(void);

#endif