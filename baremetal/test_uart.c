#define UART_BASE ((volatile unsigned char*)0x09000000)

void uart_print(const char *str) {
    while (*str) {
        *UART_BASE = *str++;
    }
}

void kernel_main(void) {
    uart_print("UART Test: If you see this, UART works!\n");
    uart_print("Type something and press Enter...\n");
    
    while(1) {
        char c = *UART_BASE;
        if (c != 0) {
            *UART_BASE = c;  // echo
            if (c == '\r') {
                *UART_BASE = '\n';
            }
        }
    }
}
