// WORKING Conversational OS for QEMU ARM64
#define UART_BASE ((volatile unsigned int*)0x09000000)
#define UART_STATUS_RX_READY (1 << 0)

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

// Check if a character is available to read
int uart_has_data(void) {
    volatile unsigned int *uart_fr = (volatile unsigned int*)0x09000018;
    return !(*uart_fr & 0x10);  // Check RX FIFO empty flag
}

// Read a character (waits if none available)
char uart_getchar(void) {
    volatile unsigned int *uart_dr = UART_BASE;
    // Wait for data to be available
    while (!uart_has_data()) {
        // Wait
    }
    return (char)(*uart_dr & 0xFF);
}

// Read line with echo and backspace
void uart_readline(char *buffer, int max_len) {
    int i = 0;
    char c;
    
    while (1) {
        c = uart_getchar();
        
        if (c == '\r' || c == '\n') {  // Enter
            buffer[i] = '\0';
            uart_putchar('\n');
            break;
        }
        else if (c == '\b' || c == 0x7f) {  // Backspace
            if (i > 0) {
                i--;
                uart_putchar('\b');
                uart_putchar(' ');
                uart_putchar('\b');
            }
        }
        else if (c >= ' ' && c <= '~') {  // Printable
            if (i < max_len - 1) {
                buffer[i++] = c;
                uart_putchar(c);
            }
        }
    }
}

// Compare two strings
int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}

// Command handler
void handle_command(const char *cmd) {
    if (cmd[0] == '\0') return;
    
    if (strcmp(cmd, "hi") == 0 || strcmp(cmd, "hello") == 0) {
        uart_print("Namaste Bhai! Welcome to your OS!\n");
    }
    else if (strcmp(cmd, "help") == 0) {
        uart_print("Commands:\n");
        uart_print("  hi, hello  - Greeting\n");
        uart_print("  name       - Tell OS name\n");
        uart_print("  time       - Show time (coming soon)\n");
        uart_print("  clear      - Clear screen\n");
        uart_print("  exit       - Halt OS\n");
        uart_print("  help       - This message\n");
    }
    else if (strcmp(cmd, "name") == 0) {
        uart_print("I don't have a name yet. You can name me!\n");
    }
    else if (strcmp(cmd, "time") == 0) {
        uart_print("Timer driver coming in Phase 2!\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        uart_print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }
    else if (strcmp(cmd, "exit") == 0) {
        uart_print("Goodbye! Halting system...\n");
        while(1);  // Stop execution
    }
    else {
        uart_print("I don't understand '");
        uart_print(cmd);
        uart_print("'. Type 'help' for commands.\n");
    }
}

void kernel_main(void) {
    char input[128];
    
    uart_print("\n\n");
    uart_print("========================================\n");
    uart_print("     YOUR CONVERSATIONAL OS v0.1\n");
    uart_print("     Built from scratch on M3 Mac\n");
    uart_print("========================================\n\n");
    uart_print("OS is running! Type 'help' to see commands.\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        handle_command(input);
    }
}
