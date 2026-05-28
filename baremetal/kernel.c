// ============================================
// CONVERSATIONAL OS v2.0 - TIME TRACKING
// Shadow AI learns what you type at each hour
// ============================================

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)

// ========== TIMER (ARM Generic Timer) ==========
static inline unsigned long long read_cntpct(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

unsigned long long last_ticks = 0;
unsigned int seconds_since_boot = 0;

void update_time(void) {
    unsigned long long now = read_cntpct();
    if (last_ticks == 0) {
        last_ticks = now;
        return;
    }
    unsigned long long diff = now - last_ticks;
    unsigned long long freq = 62500000;  // QEMU frequency
    unsigned int seconds_passed = diff / freq;
    if (seconds_passed > 0) {
        seconds_since_boot += seconds_passed;
        last_ticks = now;
    }
}

unsigned int get_current_hour(void) {
    return (seconds_since_boot / 3600) % 24;
}

// ========== UART FUNCTIONS ==========
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
    while (*UART_FR & RXFE);
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

// ========== STRING UTILITIES ==========
int str_eq(const char *a, const char *b) {
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

// ========== SHADOW AI - TIME PATTERNS ==========
#define MAX_PATTERNS 20
#define MAX_CMDLEN 32

struct time_pattern {
    char cmd[MAX_CMDLEN];
    int hour_counts[24];  // frequency per hour
    int total;
};

struct time_pattern patterns[MAX_PATTERNS];
int pattern_count = 0;

// Find or create pattern for a command
struct time_pattern* find_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++) {
        if (str_eq(patterns[i].cmd, cmd)) {
            return &patterns[i];
        }
    }
    return 0;
}

struct time_pattern* create_pattern(const char *cmd) {
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
    print("\n===== TIME PATTERNS =====\n");
    if (pattern_count == 0) {
        print("No patterns yet. Type some commands!\n");
        return;
    }
    for (int i = 0; i < pattern_count; i++) {
        print(patterns[i].cmd);
        print(" (total ");
        print_num(patterns[i].total);
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
    print("=========================\n\n");
}

// ========== COMMAND HISTORY ==========
#define MAX_HISTORY 20
char history[MAX_HISTORY][MAX_CMDLEN];
int history_count = 0;

void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }
}

void show_history(void) {
    print("\n===== HISTORY =====\n");
    for (int i = 0; i < history_count; i++) {
        print_num(i+1); print(": "); print(history[i]); print("\n");
    }
    print("==================\n\n");
}

// ========== COMMAND HANDLER ==========
char username[32] = {0};
char last_cmd[32] = {0};
int total_commands = 0;

void handle_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        print("Type something...\n");
        return;
    }
    
    // Update time and learn pattern
    update_time();
    unsigned int hour = get_current_hour();
    learn_time_pattern(cmd, hour);
    total_commands++;
    
    // Add to history
    add_to_history(cmd);
    
    // Save for next time
    int j;
    for (j = 0; cmd[j] && j < 31; j++) last_cmd[j] = cmd[j];
    last_cmd[j] = '\0';
    
    to_lower(cmd);
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        print("Namaste ");
        if (username[0]) print(username);
        print("! (hour ");
        print_num(hour);
        print(")\n");
    }
    else if (str_eq(cmd, "help")) {
        print("\n===== COMMANDS =====\n");
        print("  hi           - Greeting with time\n");
        print("  name         - Show name\n");
        print("  set name X   - Set name\n");
        print("  time         - Show uptime\n");
        print("  timepatterns - Show hourly patterns\n");
        print("  history      - Command history\n");
        print("  stats        - Show stats\n");
        print("  clear        - Clear screen\n");
        print("  exit         - Exit\n");
        print("===================\n\n");
    }
    else if (str_eq(cmd, "name")) {
        if (username[0]) { print("Name: "); print(username); print("\n"); }
        else print("No name set. Type 'set name YOURNAME'\n");
    }
    else if (cmd[0]=='s' && cmd[1]=='e' && cmd[2]=='t' && cmd[3]==' ' &&
             cmd[4]=='n' && cmd[5]=='a' && cmd[6]=='m' && cmd[7]=='e' && cmd[8]==' ') {
        char *name = cmd + 9;
        for (j = 0; name[j] && j < 31; j++) username[j] = name[j];
        username[j] = '\0';
        print("Hello "); print(username); print("!\n");
    }
    else if (str_eq(cmd, "time")) {
        print("Uptime: ");
        print_num(seconds_since_boot);
        print(" seconds (hour ");
        print_num(hour);
        print(")\n");
    }
    else if (str_eq(cmd, "timepatterns")) {
        show_time_patterns();
    }
    else if (str_eq(cmd, "history")) {
        show_history();
    }
    else if (str_eq(cmd, "stats")) {
        print("\n===== STATS =====\n");
        print("Total commands: "); print_num(total_commands); print("\n");
        print("Unique patterns: "); print_num(pattern_count); print("\n");
        print("Uptime: "); print_num(seconds_since_boot); print(" sec\n");
        print("================\n\n");
    }
    else if (str_eq(cmd, "clear")) {
        for (int i = 0; i < 30; i++) print("\n");
    }
    else if (str_eq(cmd, "exit")) {
        print("\nGoodbye! Shadow AI learned ");
        print_num(total_commands);
        print(" commands.\n");
        while(1);
    }
    else {
        print("Unknown: '"); print(cmd); print("'. Type 'help'\n");
    }
}

// ========== MAIN ==========
void kernel_main(void) {
    char input[128];
    
    last_ticks = 0;
    seconds_since_boot = 0;
    
    print("\n\n");
    print("============================================\n");
    print("     CONVERSATIONAL OS v2.0\n");
    print("     Shadow AI - Time Pattern Learning\n");
    print("============================================\n\n");
    print("Type 'help' for commands.\n");
    print("Try: hi, set name, timepatterns, stats\n\n");
    
    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
