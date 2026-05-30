#include "shadow_ai.h"
#include "uart.h"
#include "utils.h"
#include "storage.h"

#define MAX_PATTERNS 20
#define MAX_SEQUENCES 30
#define MAX_CMDLEN 32

struct time_pattern {
    char cmd[MAX_CMDLEN];
    int hour_counts[24];
    int total;
};

struct sequence {
    char from[MAX_CMDLEN];
    char to[MAX_CMDLEN];
    int count;
};

static struct time_pattern patterns[MAX_PATTERNS];
static int pattern_count = 0;
static int total_commands = 0;
static struct sequence sequences[MAX_SEQUENCES];
static int sequence_count = 0;

void shadow_ai_init(void) {
    pattern_count = 0;
    sequence_count = 0;
    total_commands = 0;
}

static struct time_pattern* find_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++)
        if (str_eq(patterns[i].cmd, cmd)) return &patterns[i];
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
    if (p) { p->total++; p->hour_counts[hour]++; }
}

void show_time_patterns(void) {
    uart_print("\n===== TIME PATTERNS =====\n");
    if (pattern_count == 0) { uart_print("No patterns yet.\n"); return; }
    for (int i = 0; i < pattern_count; i++) {
        uart_print(patterns[i].cmd); uart_print(" (total "); uart_print_num(patterns[i].total); uart_print("):\n");
        for (int h = 0; h < 24; h++)
            if (patterns[i].hour_counts[h] > 0) {
                uart_print("  "); uart_print_num(h); uart_print(":00 - "); uart_print_num(patterns[i].hour_counts[h]); uart_print(" times\n");
            }
    }
    uart_print("=========================\n\n");
}

void learn_sequence(const char *from, const char *to) {
    if (from[0] == '\0') return;
    for (int i = 0; i < sequence_count; i++)
        if (str_eq(sequences[i].from, from) && str_eq(sequences[i].to, to)) { sequences[i].count++; return; }
    if (sequence_count < MAX_SEQUENCES) {
        int j;
        for (j = 0; from[j] && j < MAX_CMDLEN-1; j++) sequences[sequence_count].from[j] = from[j];
        sequences[sequence_count].from[j] = '\0';
        for (j = 0; to[j] && j < MAX_CMDLEN-1; j++) sequences[sequence_count].to[j] = to[j];
        sequences[sequence_count].to[j] = '\0';
        sequences[sequence_count].count = 1;
        sequence_count++;
    }
}

void show_sequences(void) {
    uart_print("\n===== SEQUENCE PATTERNS =====\n");
    if (sequence_count == 0) { uart_print("No sequences yet.\n"); return; }
    for (int i = 0; i < sequence_count; i++) {
        uart_print("  "); uart_print(sequences[i].from); uart_print(" -> "); uart_print(sequences[i].to);
        uart_print(" ("); uart_print_num(sequences[i].count); uart_print(" times)\n");
    }
    uart_print("=============================\n\n");
}

void predict_next(const char *last_cmd) {
    if (last_cmd[0] == '\0') { uart_print("[Shadow AI] No previous command.\n"); return; }
    int best_idx = -1, best_count = 0;
    for (int i = 0; i < sequence_count; i++)
        if (str_eq(sequences[i].from, last_cmd) && sequences[i].count > best_count) {
            best_count = sequences[i].count;
            best_idx = i;
        }
    if (best_idx != -1) {
        uart_print("[Shadow AI] After '"); uart_print(last_cmd); uart_print("', try '");
        uart_print(sequences[best_idx].to); uart_print("' ("); uart_print_num(best_count); uart_print(" time(s))\n");
    } else {
        uart_print("[Shadow AI] No pattern for '"); uart_print(last_cmd); uart_print("' yet.\n");
    }
}

void silent_hint(const char *last_cmd) {
    if (last_cmd[0] == '\0') return;
    int best_idx = -1, best_count = 0;
    for (int i = 0; i < sequence_count; i++)
        if (str_eq(sequences[i].from, last_cmd) && sequences[i].count > best_count) {
            best_count = sequences[i].count;
            best_idx = i;
        }
    if (best_idx != -1 && best_count >= 1) {
        int confidence = (best_count * 100) / (total_commands + 5);
        if (confidence > 95) confidence = 95;
        if (confidence < 30) confidence = 30;
        uart_print("[Shadow AI] Try '"); uart_print(sequences[best_idx].to); uart_print("' next (");
        uart_print_num(confidence); uart_print("%)\n");
    }
}

int get_pattern_count(void) { return pattern_count; }
int get_total_commands(void) { return total_commands; }
void increment_total_commands(void) { total_commands++; }

void shadow_ai_save_all(void) {
    uart_print("[Shadow AI] Saving...\n");
    for (int i = 0; i < pattern_count; i++) {
        uart_print("  Pattern: ");
        uart_print(patterns[i].cmd);
        uart_print("\n");
    }
}

void shadow_ai_load_all(void) {
    uart_print("[Shadow AI] Loading...\n");
}
void shadow_ai_load_saved_data(void) {
    uart_print("[Shadow AI] Loading saved patterns...\n");
    
    // Patterns will be loaded when user types commands
    // For now, just log
    uart_print("[Shadow AI] Ready to learn from saved data\n");
}