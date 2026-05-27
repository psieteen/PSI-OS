// ============================================
// CONVERSATIONAL OS v8.0 - WITH PREDICTION
// Based on your working v7.0 + added 'predict' command
// ============================================

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)

static inline unsigned long long read_cntpct(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

unsigned long long last_ticks = 0;
unsigned int seconds_since_boot = 0;

#define MAX_PATTERNS 20
#define MAX_CMDLEN 32

struct pattern {
    char cmd[MAX_CMDLEN];
    int total_count;
    int hour_counts[24];
};

struct pattern patterns[MAX_PATTERNS];
int pattern_count = 0;

char username[32] = {0};
char history[10][MAX_CMDLEN];
int history_count = 0;
int total_commands = 0;

// ========== UTILITIES ==========

void print(const char *s) {
    while (*s) *UART_BASE = *s++;
}

void print_num(int n) {
    if (n == 0) { print("0"); return; }
    char buf[12];
    int i = 0;
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        i--;
        *UART_BASE = buf[i];
    }
}

char getchar(void) {
    while (*UART_FR & RXFE) {}
    return *UART_BASE;
}

void readline(char *buf, int max) {
    int i = 0;
    char c;
    while (i < max - 1) {
        c = getchar();
        if (c == '\r') {
            buf[i] = '\0';
            print("\n");
            return;
        } else if (c == '\b' || c == 0x7f) {
            if (i > 0) {
                i--;
                print("\b \b");
            }
        } else if (c >= ' ' && c <= '~') {
            buf[i++] = c;
            *UART_BASE = c;
        }
    }
    buf[max-1] = '\0';
}

int equal(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

void to_lower(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') *s = *s + 32;
        s++;
    }
}

char* trim(char *s) {
    while (*s == ' ') s++;
    return s;
}

// ========== TIME MANAGEMENT ==========

void update_time(void) {
    unsigned long long now = read_cntpct();
    if (last_ticks == 0) {
        last_ticks = now;
        return;
    }
    unsigned long long diff = now - last_ticks;
    unsigned long long freq = 62500000;
    unsigned int seconds_passed = diff / freq;
    if (seconds_passed > 0) {
        seconds_since_boot += seconds_passed;
        last_ticks = now;
    }
}

unsigned int get_current_hour(void) {
    return (seconds_since_boot / 3600) % 24;
}

// ========== SHADOW AI LEARNING ==========

struct pattern* find_or_create_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++) {
        if (equal(patterns[i].cmd, cmd)) return &patterns[i];
    }
    if (pattern_count < MAX_PATTERNS) {
        struct pattern *p = &patterns[pattern_count];
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) p->cmd[j] = cmd[j];
        p->cmd[j] = '\0';
        p->total_count = 0;
        for (int h = 0; h < 24; h++) p->hour_counts[h] = 0;
        pattern_count++;
        return p;
    }
    return 0;
}

void learn_pattern_with_time(const char *cmd, int hour) {
    struct pattern *p = find_or_create_pattern(cmd);
    if (p) {
        p->total_count++;
        p->hour_counts[hour]++;
    }
}

void show_time_patterns(void) {
    print("\n===== TIME-BASED PATTERNS =====\n");
    if (pattern_count == 0) {
        print("No patterns yet.\n");
    } else {
        for (int i = 0; i < pattern_count; i++) {
            print(patterns[i].cmd);
            print(" (total ");
            print_num(patterns[i].total_count);
            print("):\n");
            for (int h = 0; h < 24; h++) {
                if (patterns[i].hour_counts[h] > 0) {
                    print("  ");
                    print_num(h);
                    print(":00 - ");
                    print_num(patterns[i].hour_counts[h]);
                    print(" times\n");
                }
            }
        }
    }
    print("===============================\n\n");
}

// ========== PREDICTION (NEW) ==========

void predict_command(unsigned int hour) {
    int best_idx = -1;
    int best_freq = 0;
    int total_at_hour = 0;

    for (int i = 0; i < pattern_count; i++) {
        total_at_hour += patterns[i].hour_counts[hour];
    }

    if (total_at_hour == 0) {
        print("[Shadow AI] Not enough data for this hour yet.\n");
        return;
    }

    for (int i = 0; i < pattern_count; i++) {
        if (patterns[i].hour_counts[hour] > best_freq) {
            best_freq = patterns[i].hour_counts[hour];
            best_idx = i;
        }
    }

    if (best_idx != -1) {
        int confidence = (best_freq * 100) / total_at_hour;
        print("[Shadow AI] I predict you'll type '");
        print(patterns[best_idx].cmd);
        print("' (");
        print_num(confidence);
        print("% confidence)\n");
    }
}

// ========== COMMAND HANDLER ==========

void handle_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        print("Type something...\n");
        return;
    }

    update_time();
    unsigned int current_hour = get_current_hour();

    learn_pattern_with_time(cmd, current_hour);
    total_commands++;

    if (history_count < 10) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }

    to_lower(cmd);

    if (equal(cmd, "hi") || equal(cmd, "hello")) {
        print("Namaste ");
        if (username[0]) print(username);
        print("! (hour ");
        print_num(current_hour);
        print(")\n");
    }
    else if (equal(cmd, "help")) {
        print("\n===== COMMANDS =====\n");
        print("  hi           - Greeting\n");
        print("  name         - Show name\n");
        print("  set name X   - Set name\n");
        print("  patterns     - Simple counts\n");
        print("  timepatterns - Hourly patterns\n");
        print("  predict      - Shadow AI predicts next command\n");
        print("  time         - Uptime seconds\n");
        print("  history      - Command history\n");
        print("  stats        - Stats\n");
        print("  clear        - Clear screen\n");
        print("  exit         - Exit\n");
        print("===================\n\n");
    }
    else if (equal(cmd, "name")) {
        if (username[0]) { print("Name: "); print(username); print("\n"); }
        else print("No name set. Type 'set name YOURNAME'\n");
    }
    else if (cmd[0]=='s' && cmd[1]=='e' && cmd[2]=='t' && cmd[3]==' ' &&
             cmd[4]=='n' && cmd[5]=='a' && cmd[6]=='m' && cmd[7]=='e' && cmd[8]==' ') {
        char *name = cmd + 9;
        int j;
        for (j = 0; name[j] && j < 31; j++) username[j] = name[j];
        username[j] = '\0';
        print("Hello "); print(username); print("!\n");
    }
    else if (equal(cmd, "patterns")) {
        print("\n===== SIMPLE PATTERNS =====\n");
        for (int i = 0; i < pattern_count; i++) {
            print(patterns[i].cmd); print(": "); print_num(patterns[i].total_count); print(" times\n");
        }
        print("===========================\n\n");
    }
    else if (equal(cmd, "timepatterns")) {
        show_time_patterns();
    }
    else if (equal(cmd, "predict")) {
        predict_command(current_hour);
    }
    else if (equal(cmd, "time")) {
        print("Uptime: "); print_num(seconds_since_boot); print(" seconds\n");
    }
    else if (equal(cmd, "history")) {
        print("\n===== HISTORY =====\n");
        for (int i = 0; i < history_count; i++) { print_num(i+1); print(": "); print(history[i]); print("\n"); }
        print("==================\n\n");
    }
    else if (equal(cmd, "stats")) {
        print("\n===== STATS =====\n");
        print("Total commands: "); print_num(total_commands); print("\n");
        print("Patterns stored: "); print_num(pattern_count); print("\n");
        print("Uptime: "); print_num(seconds_since_boot); print(" sec\n");
        print("================\n\n");
    }
    else if (equal(cmd, "clear")) {
        for (int i = 0; i < 50; i++) print("\n");
    }
    else if (equal(cmd, "exit")) {
        print("\nGoodbye! Shadow AI learned ");
        print_num(total_commands);
        print(" commands.\nPress Ctrl+A then X to exit QEMU\n");
        while(1);
    }
    else {
        print("Unknown: '"); print(cmd); print("'. Type 'help'\n");
    }
}

// ========== MAIN ==========

void kernel_main(void) {
    char input[128];

    seconds_since_boot = 0;
    last_ticks = 0;

    print("\n\n");
    print("==================================================\n");
    print("     CONVERSATIONAL OS v8.0\n");
    print("     NEW: 'predict' command - Shadow AI guesses\n");
    print("==================================================\n\n");
    print("Type 'help' for commands.\n");
    print("Try: set name, hi, timepatterns, predict\n\n");

    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
