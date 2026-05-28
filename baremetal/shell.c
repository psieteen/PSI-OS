#include "shell.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "shadow_ai.h"

#define MAX_HISTORY 20
#define MAX_CMDLEN 32

static char history[MAX_HISTORY][MAX_CMDLEN];
static int history_count = 0;
static char username[32] = {0};
static char last_cmd[32] = {0};

static void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }
}

static void show_history(void) {
    uart_print("\n===== HISTORY =====\n");
    for (int i = 0; i < history_count; i++) {
        uart_print_num(i+1);
        uart_print(": ");
        uart_print(history[i]);
        uart_print("\n");
    }
    uart_print("==================\n\n");
}

static void process_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        uart_print("Type something...\n");
        return;
    }
    
    // Update time and learn pattern
    timer_update();
    unsigned int hour = timer_get_hour();
    learn_time_pattern(cmd, hour);
    increment_total_commands();
    
    // Add to history
    add_to_history(cmd);
    
    // Save for next time
    int j;
    for (j = 0; cmd[j] && j < 31; j++) last_cmd[j] = cmd[j];
    last_cmd[j] = '\0';
    
    to_lower(cmd);
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        uart_print("Namaste ");
        if (username[0]) uart_print(username);
        uart_print("! (hour ");
        uart_print_num(hour);
        uart_print(")\n");
    }
    else if (str_eq(cmd, "help")) {
        uart_print("\n===== COMMANDS =====\n");
        uart_print("  hi           - Greeting with time\n");
        uart_print("  name         - Show name\n");
        uart_print("  set name X   - Set name\n");
        uart_print("  time         - Show uptime\n");
        uart_print("  timepatterns - Show hourly patterns\n");
        uart_print("  history      - Command history\n");
        uart_print("  stats        - Show stats\n");
        uart_print("  clear        - Clear screen\n");
        uart_print("  exit         - Exit\n");
        uart_print("===================\n\n");
    }
    else if (str_eq(cmd, "name")) {
        if (username[0]) {
            uart_print("Name: ");
            uart_print(username);
            uart_print("\n");
        } else {
            uart_print("No name set. Type 'set name YOURNAME'\n");
        }
    }
    else if (cmd[0]=='s' && cmd[1]=='e' && cmd[2]=='t' && cmd[3]==' ' &&
             cmd[4]=='n' && cmd[5]=='a' && cmd[6]=='m' && cmd[7]=='e' && cmd[8]==' ') {
        char *name = cmd + 9;
        for (j = 0; name[j] && j < 31; j++) username[j] = name[j];
        username[j] = '\0';
        uart_print("Hello ");
        uart_print(username);
        uart_print("!\n");
    }
    else if (str_eq(cmd, "time")) {
        uart_print("Uptime: ");
        uart_print_num(timer_get_seconds());
        uart_print(" seconds (hour ");
        uart_print_num(hour);
        uart_print(")\n");
    }
    else if (str_eq(cmd, "timepatterns")) {
        show_time_patterns();
    }
    else if (str_eq(cmd, "history")) {
        show_history();
    }
    else if (str_eq(cmd, "stats")) {
        uart_print("\n===== STATS =====\n");
        uart_print("Total commands: ");
        uart_print_num(get_total_commands());
        uart_print("\n");
        uart_print("Unique patterns: ");
        uart_print_num(get_pattern_count());
        uart_print("\n");
        uart_print("Uptime: ");
        uart_print_num(timer_get_seconds());
        uart_print(" sec\n");
        uart_print("================\n\n");
    }
    else if (str_eq(cmd, "clear")) {
        for (int i = 0; i < 30; i++) uart_print("\n");
    }
    else if (str_eq(cmd, "exit")) {
        uart_print("\nGoodbye! Shadow AI learned ");
        uart_print_num(get_total_commands());
        uart_print(" commands.\n");
        while(1);
    }
    else {
        uart_print("Unknown: '");
        uart_print(cmd);
        uart_print("'. Type 'help'\n");
    }
}

void shell_run(void) {
    char input[128];
    
    uart_print("\n\n");
    uart_print("============================================\n");
    uart_print("     CONVERSATIONAL OS v2.0\n");
    uart_print("     Shadow AI - Time Pattern Learning\n");
    uart_print("     MODULAR ARCHITECTURE\n");
    uart_print("============================================\n\n");
    uart_print("Type 'help' for commands.\n");
    uart_print("Try: hi, set name, timepatterns, stats\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        process_command(input);
    }
}
