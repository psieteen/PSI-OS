// ============================================
// CONVERSATIONAL OS v6.0 - FINAL WORKING
// Proper UART polling with status register
// ============================================

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)  // Receive FIFO empty bit

// Print string
void print(const char *s) {
    while (*s) {
        *UART_BASE = *s++;
    }
}

// Print number
void print_num(int n) {
    if (n == 0) {
        print("0");
        return;
    }
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

// Wait for a character and read it
char getchar(void) {
    // Wait until there is data in FIFO (RXFE becomes 0)
    while (*UART_FR & RXFE) {
        // Do nothing - wait
    }
    return *UART_BASE;
}

// Read line with echo and backspace
void readline(char *buf, int max) {
    int i = 0;
    char c;
    
    while (i < max - 1) {
        c = getchar();
        
        if (c == '\r') {  // Enter
            buf[i] = '\0';
            print("\n");
            return;
        }
        else if (c == '\b' || c == 0x7f) {  // Backspace
            if (i > 0) {
                i--;
                print("\b \b");
            }
        }
        else if (c >= ' ' && c <= '~') {  // Printable
            buf[i++] = c;
            *UART_BASE = c;  // Echo
        }
    }
    buf[max-1] = '\0';
}

// String compare
int equal(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

// Convert to lowercase
void to_lower(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') {
            *s = *s + 32;
        }
        s++;
    }
}

// Trim spaces
char* trim(char *s) {
    while (*s == ' ') s++;
    return s;
}

// ========== SHADOW AI ==========
char patterns[20][32];
int pattern_count[20];
int total_patterns = 0;
int total_commands = 0;
char username[32] = {0};
char history[10][32];
int history_count = 0;

void learn_pattern(const char *cmd) {
    for (int i = 0; i < total_patterns; i++) {
        if (equal(patterns[i], cmd)) {
            pattern_count[i]++;
            return;
        }
    }
    if (total_patterns < 20) {
        int j;
        for (j = 0; cmd[j]; j++) patterns[total_patterns][j] = cmd[j];
        patterns[total_patterns][j] = '\0';
        pattern_count[total_patterns] = 1;
        total_patterns++;
    }
}

void show_patterns(void) {
    print("\n===== SHADOW AI PATTERNS =====\n");
    if (total_patterns == 0) {
        print("No patterns yet. Keep typing!\n");
    } else {
        for (int i = 0; i < total_patterns; i++) {
            print("  ");
            print(patterns[i]);
            print(": ");
            print_num(pattern_count[i]);
            print(" times\n");
        }
    }
    print("==============================\n\n");
}

// ========== COMMAND HANDLER ==========
void handle_command(char *cmd) {
    cmd = trim(cmd);
    
    if (cmd[0] == '\0') {
        print("Type something...\n");
        return;
    }
    
    // Learn pattern
    learn_pattern(cmd);
    total_commands++;
    
    // Save history
    if (history_count < 10) {
        int j;
        for (j = 0; cmd[j]; j++) history[history_count][j] = cmd[j];
        history[history_count][j] = '\0';
        history_count++;
    }
    
    // Process command (case insensitive)
    to_lower(cmd);
    
    if (equal(cmd, "hi") || equal(cmd, "hello")) {
        print("Namaste ");
        if (username[0]) print(username);
        print("!\n");
    }
    else if (equal(cmd, "help")) {
        print("\n===== COMMANDS =====\n");
        print("  hi          - Greeting\n");
        print("  name        - Show name\n");
        print("  set name X  - Set name\n");
        print("  patterns    - Show AI patterns\n");
        print("  history     - Command history\n");
        print("  stats       - Show stats\n");
        print("  clear       - Clear screen\n");
        print("  exit        - Exit\n");
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
        show_patterns();
    }
    else if (equal(cmd, "history")) {
        print("\n===== COMMAND HISTORY =====\n");
        for (int i = 0; i < history_count; i++) {
            print_num(i+1);
            print(": ");
            print(history[i]);
            print("\n");
        }
        print("===========================\n\n");
    }
    else if (equal(cmd, "stats")) {
        print("\n===== STATS =====\n");
        print("Total commands: "); print_num(total_commands); print("\n");
        print("Unique patterns: "); print_num(total_patterns); print("\n");
        print("================\n\n");
    }
    else if (equal(cmd, "clear")) {
        for (int i = 0; i < 50; i++) print("\n");
    }
    else if (equal(cmd, "exit")) {
        print("\nGoodbye! Learned ");
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
    
    print("\n\n");
    print("============================================\n");
    print("     CONVERSATIONAL OS v6.0\n");
    print("     FINALLY WORKING UART POLLING\n");
    print("     SHADOW AI ACTIVE\n");
    print("============================================\n\n");
    print("Type 'help' to begin.\n");
    print("Try: hi, set name, patterns, stats\n\n");
    
    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
