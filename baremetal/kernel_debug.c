// DEBUG VERSION - Test basic commands
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

void to_lower(char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + 32;
        }
        str++;
    }
}

char* skip_spaces(char *str) {
    while (*str == ' ') str++;
    return str;
}

void kernel_main(void) {
    char input[256];
    char *cmd;
    
    uart_print("\nDEBUG OS - Testing commands\n");
    uart_print("Try: help, hi, clear, exit\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        
        cmd = skip_spaces(input);
        to_lower(cmd);
        
        if (cmd[0] == '\0') {
            uart_print("Empty\n");
        }
        else if (str_eq(cmd, "help")) {
            uart_print("Help command works!\n");
            uart_print("Commands: help, hi, clear, exit\n");
        }
        else if (str_eq(cmd, "hi")) {
            uart_print("Hello!\n");
        }
        else if (str_eq(cmd, "clear")) {
            for(int i = 0; i < 30; i++) uart_print("\n");
        }
        else if (str_eq(cmd, "exit")) {
            uart_print("Goodbye!\n");
            while(1);
        }
        else {
            uart_print("Unknown: ");
            uart_print(cmd);
            uart_print("\n");
        }
    }
}
