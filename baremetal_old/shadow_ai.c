#include "uart.h"
#include "utils.h"

#define MAX_PATTERNS 30
#define MAX_SEQUENCES 30
#define MAX_CMDLEN 32

struct pattern {
    char cmd[MAX_CMDLEN];
    int count;
};

struct sequence {
    char from[MAX_CMDLEN];
    char to[MAX_CMDLEN];
    int count;
};

static struct pattern patterns[MAX_PATTERNS];
static int pattern_count = 0;

static struct sequence sequences[MAX_SEQUENCES];
static int sequence_count = 0;

void shadow_ai_init(void) {
    pattern_count = 0;
    sequence_count = 0;
}

void learn_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++) {
        if (str_eq(patterns[i].cmd, cmd)) {
            patterns[i].count++;
            return;
        }
    }
    if (pattern_count < MAX_PATTERNS) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) patterns[pattern_count].cmd[j] = cmd[j];
        patterns[pattern_count].cmd[j] = '\0';
        patterns[pattern_count].count = 1;
        pattern_count++;
    }
}

void learn_sequence(const char *from, const char *to) {
    if (from[0] == '\0') return;
    
    for (int i = 0; i < sequence_count; i++) {
        if (str_eq(sequences[i].from, from) && str_eq(sequences[i].to, to)) {
            sequences[i].count++;
            return;
        }
    }
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

void show_patterns(void) {
    uart_print("\n===== PATTERNS =====\n");
    if (pattern_count == 0) { uart_print("No patterns yet.\n"); return; }
    for (int i = 0; i < pattern_count; i++) {
        uart_print("  ");
        uart_print(patterns[i].cmd);
        uart_print(": ");
        uart_print_num(patterns[i].count);
        uart_print(" times\n");
    }
    uart_print("===================\n\n");
}

void show_sequences(void) {
    uart_print("\n===== SEQUENCES =====\n");
    if (sequence_count == 0) { uart_print("No sequences yet.\n"); return; }
    for (int i = 0; i < sequence_count; i++) {
        uart_print("  ");
        uart_print(sequences[i].from);
        uart_print(" -> ");
        uart_print(sequences[i].to);
        uart_print(" (");
        uart_print_num(sequences[i].count);
        uart_print(")\n");
    }
    uart_print("===================\n\n");
}

void predict(const char *last) {
    if (last[0] == '\0') {
        uart_print("[AI] No previous command.\n");
        return;
    }
    
    int best_idx = -1;
    int best_count = 0;
    
    for (int i = 0; i < sequence_count; i++) {
        if (str_eq(sequences[i].from, last)) {
            if (sequences[i].count > best_count) {
                best_count = sequences[i].count;
                best_idx = i;
            }
        }
    }
    
    if (best_idx != -1) {
        uart_print("[AI] After '");
        uart_print(last);
        uart_print("', try '");
        uart_print(sequences[best_idx].to);
        uart_print("' (");
        uart_print_num(best_count);
        uart_print(" times)\n");
    } else {
        uart_print("[AI] No pattern for '");
        uart_print(last);
        uart_print("' yet.\n");
    }
}
