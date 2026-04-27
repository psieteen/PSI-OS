// ============================================
// YOUR CONVERSATIONAL OS - FINAL WORKING VERSION
// Based on the minimal test that worked
// ============================================

#define UART_BASE ((volatile unsigned int*)0x09000000)
#define UART_FR ((volatile unsigned int*)0x09000018)
#define UART_FR_RXFE (1 << 4)

// Print string
void uart_print(const char *str) {
    while (*str) {
        *UART_BASE = *str++;
    }
}

// Read a line from user
void uart_readline(char *buffer, int max_len) {
    int i = 0;
    char c;
    
    while (i < max_len - 1) {
        // Wait for character
        while (*UART_FR & UART_FR_RXFE);
        c = *UART_BASE & 0xFF;
        
        // Enter key
        if (c == '\r') {
            buffer[i] = '\0';
            uart_print("\n");
            return;
        }
        // Backspace
        else if (c == '\b' || c == 0x7f) {
            if (i > 0) {
                i--;
                uart_print("\b \b");
            }
        }
        // Normal character
        else if (c >= ' ' && c <= '~') {
            buffer[i++] = c;
            *UART_BASE = c;
        }
    }
    buffer[max_len - 1] = '\0';
}

// Convert string to lowercase (in place)
void to_lower(char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + 32;
        }
        str++;
    }
}

// Compare two strings (case sensitive after conversion)
int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

// Check if string starts with prefix
int starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

// Skip leading spaces
char* skip_spaces(char *str) {
    while (*str == ' ') str++;
    return str;
}

// Main kernel
void kernel_main(void) {
    char input[256];
    char *cmd;
    
    uart_print("\n\n");
    uart_print("============================================\n");
    uart_print("     YOUR CONVERSATIONAL OS v1.0\n");
    uart_print("     Built from scratch on M3 Mac\n");
    uart_print("============================================\n\n");
    uart_print("Type 'help' to see what I can do.\n");
    uart_print("Try: hi, hello, name, set name YOURNAME\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        
        // Skip spaces
        cmd = skip_spaces(input);
        
        // Empty input
        if (cmd[0] == '\0') {
            uart_print("Type something...\n");
            continue;
        }
        
        // Convert to lowercase for easy comparison
        to_lower(cmd);
        
        // Handle commands
        if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
            uart_print("Namaste Bhai! Welcome to your OS!\n");
        }
        else if (str_eq(cmd, "help")) {
            uart_print("\n===== COMMANDS =====\n");
            uart_print("  hi, hello  - Greeting\n");
            uart_print("  name       - Tell OS name\n");
            uart_print("  set name X - Set your name\n");
            uart_print("  time       - Show time (soon)\n");
            uart_print("  clear      - Clear screen\n");
            uart_print("  exit       - Halt OS\n");
            uart_print("  help       - This message\n");
            uart_print("===================\n\n");
        }
        else if (str_eq(cmd, "name")) {
            uart_print("I don't have a name. Type 'set name YOURNAME'\n");
        }
        else if (starts_with(cmd, "set name ")) {
            char *name = cmd + 9;
            uart_print("Nice to meet you, ");
            uart_print(name);
            uart_print("!\n");
        }
        else if (str_eq(cmd, "time")) {
            uart_print("Timer driver coming in Phase 2!\n");
        }
        else if (str_eq(cmd, "clear")) {
            uart_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        }
        else if (str_eq(cmd, "exit") || str_eq(cmd, "quit")) {
            uart_print("Goodbye! Halting system...\n");
            while(1);
        }
        else {
            uart_print("I don't understand '");
            uart_print(cmd);
            uart_print("'. Type 'help'\n");
        }
    }
}
