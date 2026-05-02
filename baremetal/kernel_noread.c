#define UART_BASE ((volatile unsigned int*)0x09000000)

void uart_print(const char *str) {
    while (*str) {
        *UART_BASE = *str++;
    }
}

void kernel_main(void) {
    uart_print("\nNO INPUT TEST - Just printing\n");
    uart_print("If you see this, printing works\n");
    uart_print("help\n");
    uart_print("help command printed\n");
    uart_print("If you see all this, the problem is in uart_readline\n");
    while(1);
}
