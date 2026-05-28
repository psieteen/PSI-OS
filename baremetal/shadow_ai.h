#ifndef SHADOW_AI_H
#define SHADOW_AI_H

void shadow_ai_init(void);
void learn_time_pattern(const char *cmd, int hour);
void show_time_patterns(void);
int get_pattern_count(void);
int get_total_commands(void);
void increment_total_commands(void);

#endif
