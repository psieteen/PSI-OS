#define UART_BASE ((volatile unsigned int*)0x09000000)
#define UART_FR ((volatile unsigned int*)0x09000018)
#define UART_FR_RXFE (1 << 4)

void uart_print(const char *str) {
    while (*str) {
        *UART_BASE = *str++;
    }
}

void uart_readline(char *buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        while (*UART_FR & UART_FR_RXFE);
        c = *UART_BASE & 0xFF;
        if (c == '\r') {
            buffer[i] = '\0';
            uart_print("\n");
            return;
        }
        else if (c == '\b' || c == 0x7f) {
            if (i > 0) {
                i--;
                uart_print("\b \b");
            }
        }
        else if (c >= ' ' && c <= '~') {
            buffer[i++] = c;
            *UART_BASE = c;
        }
    }
    buffer[max_len - 1] = '\0';
}

int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void kernel_main(void) {
    char input[64];
    
    uart_print("\nMINIMAL TEST - Type 'help'\n");
    uart_print("If this hangs, the problem is in uart_readline\n\n");
    
    while (1) {
        uart_print("> ");
        uart_readline(input, sizeof(input));
        
        uart_print("You typed: ");
        uart_print(input);
        uart_print("\n");
        
        if (str_eq(input, "help")) {
            uart_print("Help command recognized!\n");
        }
        else if (str_eq(input, "exit")) {
            uart_print("Exiting...\n");
            while(1);
        }
    }
}
