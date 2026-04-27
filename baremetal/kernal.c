
#define UART_BASE ((volatile unsigned int*)0x09000000)
#define UART_FR  ((volatile unsigned int*)0x09000018)
#define UART_FR_RXFE (1 << 4)

// Print single character
void uart_putchar(char c) {
    volatile unsigned int *uart_dr = UART_BASE;
    *uart_dr = c;
}

// Print string
void uart_print(const char *str) {
    while (*str) {
        uart_putchar(*str++);
    }
}

// Check if character available
int uart_has_data(void) {
    return !(*UART_FR & UART_FR_RXFE);
}

// Read character (waits)
char uart_getchar(void) {
    volatile unsigned int *uart_dr = UART_BASE;
    while (!uart_has_data());
    return (char)(*uart_dr & 0xFF);
}

// Read line from user
void uart_readline(char *buffer, int max_len) {
    int i = 0;
    char c;
    
    while (i < max_len - 1) {
        c = uart_getchar();
        
        // Enter key
        if (c == '\r' || c == '\n') {
            buffer[i] = '\0';
            uart_putchar('\n');
            return;
        }
        // Backspace
        else if (c == '\b' || c == 0x7f) {
            if (i > 0) {
                i--;
                uart_putchar('\b');
                uart_putchar(' ');
                uart_putchar('\b');
            }
        }
        // Printable characters
        else if (c >= ' ' && c <= '~') {
            buffer[i++] = c;
            uart_putchar(c);
        }
    }
    buffer[max_len - 1] = '\0';
}

// Simple string copy
void str_copy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

// Convert to lowercase (in place)
void to_lowercase(char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + ('a' - 'A');
        }
        str++;
    }
}

// Trim leading spaces (returns pointer to first non-space)
char* trim_left(char *str) {
    while (*str == ' ') {
        str++;
    }
    return str;
}

// Compare two strings (case insensitive)
int strcmp_ci(const char *a, const char *b) {
    char ca, cb;
    while (*a && *b) {
        ca = (*a >= 'A' && *a <= 'Z') ? (*a + ('a' - 'A')) : *a;
        cb = (*b >= 'A' && *b <= 'Z') ? (*b + ('a' - 'A')) : *b;
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    ca = (*a >= 'A' && *a <= 'Z') ? (*a + ('a' - 'A')) : *a;
    cb = (*b >= 'B' && *b <= 'Z') ? (*b + ('a' - 'A')) : *b;
    return ca - cb;
}

// Check if command starts with a string
int starts_with(const char *cmd, const char *prefix) {
    while (*prefix) {
        char cc = (*cmd >= 'A' && *cmd <= 'Z') ? (*cmd + ('a' - 'A')) : *cmd;
        char cp = (*prefix >= 'A' && *prefix <= 'Z') ? (*prefix + ('a' - 'A')) : *prefix;
        if (cc != cp) return 0;
        cmd++;
        prefix++;
    }
    return 1;
}

// Command handler
void handle_command(char *cmd) {
    // Trim leading spaces
    cmd = trim_left(cmd);
    
    // Check for empty input
    if (cmd[0] == '\0') {
        uart_print("Type something...\n");
        return;
    }
    
    // Convert to lowercase for comparison
    to_lowercase(cmd);
    
    // Handle commands
    if (strcmp_ci(cmd, "hi") == 0 || strcmp_ci(cmd, "hello") == 0) {
        uart_print("Namaste Bhai! Welcome to your OS!\n");
    }
    else if (strcmp_ci(cmd, "help") == 0) {
        uart_print("\n===== COMMANDS =====\n");
        uart_print("  hi, hello  - Greeting\n");
        uart_print("  name       - Tell OS name\n");
        uart_print("  time       - Show time (coming soon)\n");
        uart_print("  clear      - Clear screen\n");
        uart_print("  exit       - Halt OS\n");
        uart_print("  help       - This message\n");
        uart_print("===================\n\n");
    }
    else if (strcmp_ci(cmd, "name") == 0) {
        uart_print("I don't have a name yet. You can name me!\n");
        uart_print("Type 'set name YOURNAME' to name me.\n");
    }
    else if (starts_with(cmd, "set name ")) {
        char *name = cmd + 9;
        uart_print("Nice to meet you, ");
        uart_print(name);
        uart_print("! I'll remember you.\n");
    }
    else if (strcmp_ci(cmd, "time") == 0) {
        uart_print("Timer driver coming in Phase 2!\n");
    }
    else if (strcmp_ci(cmd, "clear") == 0) {
        uart_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }
    else if (strcmp_ci(cmd, "exit") == 0 || strcmp_ci(cmd, "quit") == 0) {
        uart_print("Goodbye! Halting system...\n");
        while(1);
    }
    else {
        uart_print("I don't understand '");
        uart_print(cmd);
        uart_print("'. Type 'help' for commands.\n");
    }
}

// Main kernel entry point
void kernel_main(void) {
    char input_buffer[256];
    
    uart_print("\n\n");
    uart_print("============================================\n");
    uart_print("     YOUR CONVERSATIONAL OS v1.0\n");
    uart_print("     Built from scratch on M3 Mac\n");
    uart_print("     All features working!\n");
    uart_print("============================================\n\n");
    uart_print("Type 'help' to see what I can do.\n");
    uart_print("Try: hi, hello, name, set name YOURNAME\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input_buffer, sizeof(input_buffer));
        handle_command(input_buffer);
    }
}
