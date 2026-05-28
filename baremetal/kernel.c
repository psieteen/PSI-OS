// ============================================
// CONVERSATIONAL OS - FRESH START
// Simple, clean, working
// ============================================

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)

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

// ========== SHADOW AI (SIMPLE START) ==========
#define MAX_HISTORY 20
char history[MAX_HISTORY][32];
int history_count = 0;
char last_cmd[32] = {0};

void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        int j;
        for (j = 0; cmd[j] && j < 31; j++) history[history_count][j] = cmd[j];
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

void handle_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        print("Type something...\n");
        return;
    }
    
    add_to_history(cmd);
    
    // Save for next time
    int j;
    for (j = 0; cmd[j] && j < 31; j++) last_cmd[j] = cmd[j];
    last_cmd[j] = '\0';
    
    to_lower(cmd);
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        print("Namaste ");
        if (username[0]) print(username);
        print("!\n");
    }
    else if (str_eq(cmd, "help")) {
        print("\n===== COMMANDS =====\n");
        print("  hi           - Greeting\n");
        print("  name         - Show name\n");
        print("  set name X   - Set name\n");
        print("  history      - Show command history\n");
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
    else if (str_eq(cmd, "history")) {
        show_history();
    }
    else if (str_eq(cmd, "clear")) {
        for (int i = 0; i < 30; i++) print("\n");
    }
    else if (str_eq(cmd, "exit")) {
        print("Goodbye!\n");
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
    print("========================================\n");
    print("     CONVERSATIONAL OS - FRESH START\n");
    print("     Simple. Clean. Working.\n");
    print("========================================\n\n");
    print("Type 'help' to begin.\n\n");
    
    while (1) {
        print(">> ");
        readline(input, sizeof(input));
        handle_command(input);
    }
}
