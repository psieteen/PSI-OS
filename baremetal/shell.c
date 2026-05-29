#include "shell.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "shadow_ai.h"
#include "storage.h"

#define MAX_HISTORY 20
#define MAX_CMDLEN 64

static char history[MAX_HISTORY][MAX_CMDLEN];
static int history_count = 0;
static int history_pos = 0;
static char username[32] = {0};
static char last_cmd[32] = {0};

static void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }
    history_pos = history_count;
}

static void show_history(void) {
    uart_print("\n===== HISTORY =====\n");
    for (int i = 0; i < history_count; i++) {
        uart_print_num(i+1); uart_print(": "); uart_print(history[i]); uart_print("\n");
    }
    uart_print("==================\n\n");
}

static void handle_history_navigation(char *buffer, int *len, int key) {
    if (key == 'A') {
        if (history_pos > 0) {
            while (*len > 0) { uart_print("\b \b"); (*len)--; }
            history_pos--;
            int j;
            for (j = 0; history[history_pos][j] && j < MAX_CMDLEN-1; j++) {
                buffer[j] = history[history_pos][j];
                uart_print_char(buffer[j]);
            }
            buffer[j] = '\0'; *len = j;
        }
    } else if (key == 'B') {
        if (history_pos < history_count - 1) {
            while (*len > 0) { uart_print("\b \b"); (*len)--; }
            history_pos++;
            int j;
            for (j = 0; history[history_pos][j] && j < MAX_CMDLEN-1; j++) {
                buffer[j] = history[history_pos][j];
                uart_print_char(buffer[j]);
            }
            buffer[j] = '\0'; *len = j;
        } else if (history_pos == history_count - 1) {
            while (*len > 0) { uart_print("\b \b"); (*len)--; }
            history_pos = history_count;
            buffer[0] = '\0'; *len = 0;
        }
    }
}

static void readline_with_history(char *buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        c = uart_getchar();
        if (c == '\r') { buffer[i] = '\0'; uart_print("\n"); return; }
        else if (c == '\b' || c == 0x7f) { if (i > 0) { i--; uart_print("\b \b"); } }
        else if (c == 0x1b) {
            c = uart_getchar();
            if (c == '[') {
                c = uart_getchar();
                if (c == 'A' || c == 'B') handle_history_navigation(buffer, &i, c);
            }
        }
        else if (c >= ' ' && c <= '~') { buffer[i++] = c; uart_print_char(c); }
    }
    buffer[max_len-1] = '\0';
}

static void show_aliases(void) {
    uart_print("\n===== COMMAND ALIASES =====\n");
    uart_print("  h  -> help\n  p  -> predict\n  s  -> sequences\n  t  -> time\n");
    uart_print("  tp -> timepatterns\n  hist -> history\n  st -> stats\n  cl -> clear\n");
    uart_print("==========================\n\n");
}

static void process_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') { uart_print("Type something...\n"); return; }
    
    timer_update();
    unsigned int hour = timer_get_hour();
    
    if (last_cmd[0] != '\0') learn_sequence(last_cmd, cmd);
    learn_time_pattern(cmd, hour);
    increment_total_commands();
    add_to_history(cmd);
    
    int j;
    for (j = 0; cmd[j] && j < 31; j++) last_cmd[j] = cmd[j];
    last_cmd[j] = '\0';
    
    char original_cmd[MAX_CMDLEN];
    for (j = 0; cmd[j]; j++) original_cmd[j] = cmd[j];
    original_cmd[j] = '\0';
    
    to_lower(cmd);
    
    if (str_eq(cmd, "h")) cmd = "help";
    else if (str_eq(cmd, "p")) cmd = "predict";
    else if (str_eq(cmd, "s")) cmd = "sequences";
    else if (str_eq(cmd, "t")) cmd = "time";
    else if (str_eq(cmd, "tp")) cmd = "timepatterns";
    else if (str_eq(cmd, "hist")) cmd = "history";
    else if (str_eq(cmd, "st")) cmd = "stats";
    else if (str_eq(cmd, "cl")) cmd = "clear";
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        uart_print("Namaste "); if (username[0]) uart_print(username); uart_print("!\n");
    }
    else if (str_eq(cmd, "help")) {
        uart_print("\n===== COMMANDS =====\n");
        uart_print("  hi, hello    - Greeting\n");
        uart_print("  name         - Show name\n");
        uart_print("  set name X   - Set name\n");
        uart_print("  time         - Show real date & time\n");
        uart_print("  uptime       - Show system uptime\n");
        uart_print("  timepatterns - Hourly patterns\n");
        uart_print("  sequences    - Show command sequences\n");
        uart_print("  predict      - Predict next command\n");
        uart_print("  history      - Command history\n");
        uart_print("  stats        - Show stats\n");
        uart_print("  echo X       - Echo text\n");
        uart_print("  aliases      - Show command aliases\n");
        uart_print("  save         - Save data\n");
        uart_print("  clear        - Clear screen\n");
        uart_print("  exit         - Exit\n");
        uart_print("===================\n\n");
    }
    else if (str_eq(cmd, "aliases")) { show_aliases(); }
    else if (str_eq(cmd, "name")) {
        if (username[0]) { uart_print("Name: "); uart_print(username); uart_print("\n"); }
        else uart_print("No name set. Type 'set name YOURNAME'\n");
    }
    else if (cmd[0]=='s' && cmd[1]=='e' && cmd[2]=='t' && cmd[3]==' ' &&
             cmd[4]=='n' && cmd[5]=='a' && cmd[6]=='m' && cmd[7]=='e' && cmd[8]==' ') {
        char *name = cmd + 9;
        for (j = 0; name[j] && j < 31; j++) username[j] = name[j];
        username[j] = '\0';
        uart_print("Hello "); uart_print(username); uart_print("!\n");
        storage_save_name(username);
    }
    else if (str_eq(cmd, "time")) {
        unsigned int year, month, day, hour, min, sec;
        rtc_get_datetime(&year, &month, &day, &hour, &min, &sec);
        uart_print_num(day); uart_print("/"); uart_print_num(month); uart_print("/"); uart_print_num(year);
        uart_print(" "); uart_print_num(hour); uart_print(":");
        if (min < 10) uart_print("0"); uart_print_num(min); uart_print(":");
        if (sec < 10) uart_print("0"); uart_print_num(sec); uart_print("\n");
    }
    else if (str_eq(cmd, "uptime")) {
        uart_print("Uptime: "); uart_print_num(timer_get_seconds()); uart_print(" seconds\n");
    }
    else if (starts_with(cmd, "echo ")) {
        const char *msg = original_cmd + 5;
        uart_print(msg); uart_print("\n");
    }
    else if (str_eq(cmd, "timepatterns")) { show_time_patterns(); }
    else if (str_eq(cmd, "sequences")) { show_sequences(); }
    else if (str_eq(cmd, "predict")) { predict_next(last_cmd); }
    else if (str_eq(cmd, "history")) { show_history(); }
    else if (str_eq(cmd, "stats")) {
        uart_print("\n===== STATS =====\n");
        uart_print("Total commands: "); uart_print_num(get_total_commands()); uart_print("\n");
        uart_print("Time patterns: "); uart_print_num(get_pattern_count()); uart_print("\n");
        uart_print("Uptime: "); uart_print_num(timer_get_seconds()); uart_print(" sec\n");
        uart_print("================\n\n");
    }
    else if (str_eq(cmd, "save")) {
        shadow_ai_save_all();
        storage_save_name(username);
        uart_print("Save complete!\n");
    }
    else if (str_eq(cmd, "clear")) { for (int i = 0; i < 30; i++) uart_print("\n"); }
    else if (str_eq(cmd, "exit")) {
        uart_print("\nSaving...\n");
        shadow_ai_save_all();
        storage_save_name(username);
        uart_print("Goodbye!\n");
        while(1);
    }
    else {
        uart_print("Unknown: '"); uart_print(cmd); uart_print("'. Type 'help'\n");
    }
    
    if (!str_eq(cmd, "predict")) silent_hint(last_cmd);
}

void shell_run(void) {
    char input[MAX_CMDLEN];
    char saved_name[32] = {0};
    
    uart_print("\n\n");
    uart_print("============================================\n");
    uart_print("     PSIOSTALKS v1.0\n");
    uart_print("     Conversational OS with Storage\n");
    uart_print("============================================\n\n");
    
    if (storage_load_name(saved_name, sizeof(saved_name))) {
        int j;
        for (j = 0; saved_name[j] && j < 31; j++) username[j] = saved_name[j];
        username[j] = '\0';
        uart_print("Welcome back, "); uart_print(username); uart_print("!\n");
    }
    
    uart_print("\nType 'help' for commands.\n");
    uart_print("Type 'save' to save your data.\n\n");
    
    while (1) {
        uart_print(">> ");
        readline_with_history(input, sizeof(input));
        process_command(input);
    }
}