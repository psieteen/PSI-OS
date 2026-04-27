#define UART_BASE ((volatile unsigned char*)0x09000000)

// Write a single character to the screen
void uart_putchar(char c) {
    *UART_BASE = c;
}

// Print a string to the screen
void uart_print(const char *str) {
    while (*str) {
        uart_putchar(*str++);
    }
}

// Your kernel's entry point
void kernel_main(void) {
    uart_print("Hello from MY OS running on BARE METAL!\n");
    uart_print("No macOS. No Linux. Just me and the hardware.\n");
    uart_print("Your conversational AI OS has booted.\n");
    
    // Your OS will live here forever
    while(1) {
        // Eventually: chat with user, run AI, adapt silently
        // For now, just do nothing
    }
}
