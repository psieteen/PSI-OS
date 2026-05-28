#include "shadow_ai.h"
#include "uart.h"
#include "utils.h"

#define MAX_PATTERNS 20
#define MAX_CMDLEN 32

struct time_pattern {
    char cmd[MAX_CMDLEN];
    int hour_counts[24];
    int total;
};

static struct time_pattern patterns[MAX_PATTERNS];
static int pattern_count = 0;
static int total_commands = 0;

void shadow_ai_init(void) {
    pattern_count = 0;
    total_commands = 0;
}

static struct time_pattern* find_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++) {
        if (str_eq(patterns[i].cmd, cmd)) {
            return &patterns[i];
        }
    }
    return 0;
}

static struct time_pattern* create_pattern(const char *cmd) {
    if (pattern_count >= MAX_PATTERNS) return 0;
    struct time_pattern *p = &patterns[pattern_count];
    int j;
    for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) p->cmd[j] = cmd[j];
    p->cmd[j] = '\0';
    p->total = 0;
    for (int h = 0; h < 24; h++) p->hour_counts[h] = 0;
    pattern_count++;
    return p;
}

void learn_time_pattern(const char *cmd, int hour) {
    struct time_pattern *p = find_pattern(cmd);
    if (!p) p = create_pattern(cmd);
    if (p) {
        p->total++;
        p->hour_counts[hour]++;
    }
}

void show_time_patterns(void) {
    uart_print("\n===== TIME PATTERNS =====\n");
    if (pattern_count == 0) {
        uart_print("No patterns yet. Type some commands!\n");
        return;
    }
    for (int i = 0; i < pattern_count; i++) {
        uart_print(patterns[i].cmd);
        uart_print(" (total ");
        uart_print_num(patterns[i].total);
        uart_print("):\n");
        for (int h = 0; h < 24; h++) {
            if (patterns[i].hour_counts[h] > 0) {
                uart_print("  ");
                uart_print_num(h);
                uart_print(":00 - ");
                uart_print_num(patterns[i].hour_counts[h]);
                uart_print(" times\n");
            }
        }
    }
    uart_print("=========================\n\n");
}

int get_pattern_count(void) {
    return pattern_count;
}

int get_total_commands(void) {
    return total_commands;
}

void increment_total_commands(void) {
    total_commands++;
}
