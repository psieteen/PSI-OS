// ============================================
// CONVERSATIONAL OS v7.0 - REAL TIME & TIME-BASED SHADOW AI
// Adds: ARM Generic Timer, timestamps, time-of-day patterns
// ============================================

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)

// ARM Generic Timer registers (for QEMU)
#define CNTPCT_EL0  ((volatile unsigned long long*)0x3FF0000)  // Actually, need assembly; but QEMU maps it

// Simplified: we'll use a function to read the counter
static inline unsigned long long read_cntpct(void) {
    unsigned long long val;
    asm volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

// Global time state (for demo, we'll keep seconds since boot)
unsigned long long boot_time_ticks = 0;
unsigned int seconds_since_boot = 0;
unsigned int last_second = 0;

// Time-based patterns: store command counts per hour (0-23)
int time_patterns[24][20];  // hour -> command index count (simplified: we'll just store counts per command string)
// We'll instead use a simpler approach: log commands with timestamp

// For simplicity, we'll enhance existing patterns array with time info
#define MAX_PATTERNS 20
#define MAX_CMDLEN 32

struct pattern {
    char cmd[MAX_CMDLEN];
    int total_count;
    int hour_counts[24];    // frequency per hour
};

struct pattern patterns[MAX_PATTERNS];
int pattern_count = 0;

// History and other global state
char username[32] = {0};
char history[10][MAX_CMDLEN];
int history_count = 0;
int total_commands = 0;

// Forward declarations
void print(const char *s);
void print_num(int n);
void print_llu(unsigned long long n);
char getchar(void);
void readline(char *buf, int max);
int equal(const char *a, const char *b);
void to_lower(char *s);
char* trim(char *s);
void learn_pattern_with_time(const char *cmd, int hour);
void show_time_patterns(void);
void update_time(void);
void handle_command(char *cmd);

// ========== UTILITY FUNCTIONS ==========

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

void print_llu(unsigned long long n) {
    if (n == 0) { print("0"); return; }
    char buf[20];
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

unsigned int get_current_hour(void) {
    // Use seconds since boot, assume boot at 00:00:00 for demo
    // Real implementation would read RTC, but for now: hour = (seconds_since_boot / 3600) % 24
    return (seconds_since_boot / 3600) % 24;
}

void update_time(void) {
    unsigned long long now = read_cntpct();
    // QEMU's counter frequency is typically 62.5 MHz (62,500,000)
    // We'll assume it's 62.5 MHz for conversion; but we can calculate more robustly by calibrating.
    // For simplicity, we'll just increment seconds_since_boot every time we see a change.
    // But better: use a simple timer tick. Let's do a lazy update: check if we crossed a second.
    static unsigned long long last_ticks = 0;
    if (last_ticks == 0) {
        last_ticks = now;
        return;
    }
    // Assume frequency 62.5 MHz (typical for QEMU virt)
    // Actually, we can compute by reading CNTFRQ_EL0 (but that's tricky in EL0? We are in EL1).
    // Simpler: assume 62.5 MHz. For demonstration, we'll just increment each loop call.
    // But that's inaccurate. Let's just use a counter that increments per command.
    // However, to keep it simple, we'll manually update seconds_since_boot based on the difference.
    unsigned long long diff = now - last_ticks;
    // approx freq: 62,500,000 ticks per second.
    unsigned long long freq = 62500000;
    unsigned int seconds_passed = diff / freq;
    if (seconds_passed > 0) {
        seconds_since_boot += seconds_passed;
        last_ticks = now;
    }
}

// ========== SHADOW AI - TIME-BASED PATTERNS ==========

// Find or create pattern for command
struct pattern* find_or_create_pattern(const char *cmd) {
    for (int i = 0; i < pattern_count; i++) {
        if (equal(patterns[i].cmd, cmd)) {
            return &patterns[i];
        }
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
    print("\n===== TIME-BASED SHADOW AI PATTERNS =====\n");
    if (pattern_count == 0) {
        print("No patterns yet. Type commands!\n");
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
    print("=========================================\n\n");
}

// ========== COMMAND HANDLER ==========

void handle_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        print("Type something...\n");
        return;
    }

    // Update time before processing command
    update_time();
    unsigned int current_hour = get_current_hour();

    // Learn pattern with timestamp
    learn_pattern_with_time(cmd, current_hour);
    total_commands++;

    // Save history
    if (history_count < 10) {
        int j;
        for (j = 0; cmd[j] && j < MAX_CMDLEN-1; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }

    // Process command (case insensitive)
    to_lower(cmd);

    if (equal(cmd, "hi") || equal(cmd, "hello")) {
        print("Namaste ");
        if (username[0]) print(username);
        print("! ");
        print("Current hour: ");
        print_num(current_hour);
        print("\n");
    }
    else if (equal(cmd, "help")) {
        print("\n===== COMMANDS =====\n");
        print("  hi           - Greeting with time\n");
        print("  name         - Show name\n");
        print("  set name X   - Set name\n");
        print("  patterns     - Show simple patterns (counts)\n");
        print("  timepatterns - Show time-based patterns (hourly)\n");
        print("  time         - Show uptime in seconds\n");
        print("  history      - Command history\n");
        print("  stats        - Show stats\n");
        print("  clear        - Clear screen\n");
        print("  exit         - Exit OS\n");
        print("===================\n\n");
    }
    else if (equal(cmd, "name")) {
        if (username[0]) {
            print("Name: "); print(username); print("\n");
        } else {
            print("No name set. Type 'set name YOURNAME'\n");
        }
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
        // Show simple pattern counts (original)
        print("\n===== SIMPLE PATTERNS =====\n");
        for (int i = 0; i < pattern_count; i++) {
            print(patterns[i].cmd);
            print(": ");
            print_num(patterns[i].total_count);
            print(" times\n");
        }
        print("===========================\n\n");
    }
    else if (equal(cmd, "timepatterns")) {
        show_time_patterns();
    }
    else if (equal(cmd, "time")) {
        print("Uptime seconds: ");
        print_num(seconds_since_boot);
        print("\n");
    }
    else if (equal(cmd, "history")) {
        print("\n===== COMMAND HISTORY =====\n");
        for (int i = 0; i < history_count; i++) {
            print_num(i+1); print(": "); print(history[i]); print("\n");
        }
        print("===========================\n\n");
    }
    else if (equal(cmd, "stats")) {
        print("\n===== STATS =====\n");
        print("Total commands: "); print_num(total_commands); print("\n");
        print("Total patterns: "); print_num(pattern_count); print("\n");
        print("Uptime: "); print_num(seconds_since_boot); print(" sec\n");
        print("================\n\n");
    }
    else if (equal(cmd, "clear")) {
        for (int i = 0; i < 50; i++) print("\n");
    }
    else if (equal(cmd, "exit")) {
        print("\nGoodbye! Shadow AI learned ");
        print_num(total_commands);
        print(" commands.\n");
        print("Press Ctrl+A then X to exit QEMU\n");
        while(1);
    }
    else {
        print("Unknown: '"); print(cmd); print("'. Type 'help'\n");
    }
}

// ========== MAIN ==========

void kernel_main(void) {
    char input[128];

    // Initialize timer
    boot_time_ticks = read_cntpct();
    seconds_since_boot = 0;
    last_second = 0;

    print("\n\n");
    print("=================================================\n");
    print("     CONVERSATIONAL OS v7.0 - REAL TIME + TIME-BASED AI\n");
    print("     Shadow AI now learns what you do at each hour\n");
    print("=================================================\n\n");
    print("Type 'help' to begin.\n");
    print("Try: hi, set name, time, timepatterns\n\n");

    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
