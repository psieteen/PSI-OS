
// ============================================
// CONVERSATIONAL OS v9.0 - SEQUENCE PREDICTION
// Shadow AI learns: after X comes Y
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

// Time-based patterns
#define MAX_PATTERNS 20
#define MAX_SEQUENCES 30
#define MAX_CMDLEN 32

struct pattern {
    char cmd[MAX_CMDLEN];
    int total_count;
    int hour_counts[24];
};

struct sequence {
    char from[MAX_CMDLEN];
    char to[MAX_CMDLEN];
    int count;
};

struct pattern patterns[MAX_PATTERNS];
int pattern_count = 0;

struct sequence sequences[MAX_SEQUENCES];
int sequence_count = 0;

char last_command[MAX_CMDLEN] = {0};
char username[32] = {0};
char history[20][MAX_CMDLEN];
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

// ========== SHADOW AI - TIME PATTERNS ==========

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
    print("\n===== TIME PATTERNS =====\n");
    if (pattern_count == 0) { print("No patterns yet.\n"); return; }
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
    print("=========================\n\n");
}

// ========== SHADOW AI - SEQUENCE PATTERNS (NEW) ==========

void learn_sequence(const char *from, const char *to) {
    if (from[0] == '\0') return;
    
    for (int i = 0; i < sequence_count; i++) {
        if (equal(sequences[i].from, from) && equal(sequences[i].to, to)) {
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

void show_sequences(void) {
    print("\n===== SEQUENCE PATTERNS =====\n");
    if (sequence_count == 0) { print("No sequences yet.\n"); return; }
    for (int i = 0; i < sequence_count; i++) {
        print("  ");
        print(sequences[i].from);
        print(" -> ");
        print(sequences[i].to);
        print(" (");
        print_num(sequences[i].count);
        print(" times)\n");
    }
    print("=============================\n\n");
}

// ========== SMART PREDICTION (Time + Sequence) ==========

void predict_smart(unsigned int hour, const char *last) {
    int best_seq_idx = -1;
    int best_seq_count = 0;
    int best_time_idx = -1;
    int best_time_freq = 0;
    int total_at_hour = 0;
    
    // First, try sequence prediction based on last command
    if (last[0] != '\0') {
        for (int i = 0; i < sequence_count; i++) {
            if (equal(sequences[i].from, last)) {
                if (sequences[i].count > best_seq_count) {
                    best_seq_count = sequences[i].count;
                    best_seq_idx = i;
                }
            }
        }
    }
    
    // Also compute time-based prediction
    for (int i = 0; i < pattern_count; i++) {
        total_at_hour += patterns[i].hour_counts[hour];
    }
    for (int i = 0; i < pattern_count; i++) {
        if (patterns[i].hour_counts[hour] > best_time_freq) {
            best_time_freq = patterns[i].hour_counts[hour];
            best_time_idx = i;
        }
    }
    
    // Decide which prediction to show
    if (best_seq_idx != -1 && best_seq_count >= 2) {
        int confidence = (best_seq_count * 100) / (total_commands + 1);
        if (confidence > 100) confidence = 95;
        print("[Shadow AI - Sequence] After '");
        print(last);
        print("', you usually type '");
        print(sequences[best_seq_idx].to);
        print("' (");
        print_num(confidence);
        print("%)\n");
    } 
    else if (best_time_idx != -1 && total_at_hour > 0) {
        int confidence = (best_time_freq * 100) / total_at_hour;
        print("[Shadow AI - Time] At hour ");
        print_num(hour);
        print(", you usually type '");
        print(patterns[best_time_idx].cmd);
        print("' (");
        print_num(confidence);
        print("%)\n");
    }
    else {
        print("[Shadow AI] Not enough data to predict yet.\n");
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

    // Learn sequence (from last command to this command)
    if (last_command[0] != '\0') {
        learn_sequence(last_command, cmd);
    }
    
    // Learn time pattern
    learn_pattern_with_time(cmd, current_hour);
    total_commands++;

    // Save history
    if (history_count < 20) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }
    
    // Save as last command for next sequence
    int j;
    for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) last_command[j] = cmd[j];
    last_command[j] = '\0';
    
    // Process command (case insensitive)
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
        print("  sequences    - Show sequence patterns (NEW)\n");
        print("  predict      - Smart prediction (time+sequence)\n");
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
    else if (equal(cmd, "sequences")) {
        show_sequences();
    }
    else if (equal(cmd, "predict")) {
        predict_smart(current_hour, last_command);
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
        print("Time patterns: "); print_num(pattern_count); print("\n");
        print("Sequence patterns: "); print_num(sequence_count); print("\n");
        print("Uptime: "); print_num(seconds_since_boot); print(" sec\n");
        print("================\n\n");
    }
    else if (equal(cmd, "clear")) {
        for (int i = 0; i < 50; i++) print("\n");
    }
    else if (equal(cmd, "exit")) {
        print("\nGoodbye! Shadow AI learned ");
        print_num(total_commands);
        print(" commands and ");
        print_num(sequence_count);
        print(" sequences.\nPress Ctrl+A then X to exit QEMU\n");
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
    last_command[0] = '\0';

    print("\n\n");
    print("==================================================\n");
    print("     CONVERSATIONAL OS v9.0\n");
    print("     SHADOW AI NOW LEARNS SEQUENCES\n");
    print("     'predict' uses time + last command\n");
    print("==================================================\n\n");
    print("New command: 'sequences' - see what Shadow AI learned\n");
    print("Try: hi, patterns, predict, sequences\n\n");

    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
