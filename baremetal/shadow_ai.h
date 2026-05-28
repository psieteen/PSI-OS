#ifndef SHADOW_AI_H
#define SHADOW_AI_H

void shadow_ai_init(void);
void learn_time_pattern(const char *cmd, int hour);
void show_time_patterns(void);
void learn_sequence(const char *from, const char *to);
void show_sequences(void);
void predict_next(const char *last_cmd);
int get_pattern_count(void);
int get_total_commands(void);
void increment_total_commands(void);

#endif