// Add this to handle_command() - replace the exit section

else if (str_eq(cmd, "exit") || str_eq(cmd, "quit")) {
    uart_print("Goodbye! Halting system...\n");
    uart_print("[Shadow AI] Learned ");
    // print number of commands
    int temp = history_count;
    char buf[10];
    int i = 0;
    if (temp == 0) {
        buf[i++] = '0';
    } else {
        while (temp > 0) {
            buf[i++] = '0' + (temp % 10);
            temp /= 10;
        }
    }
    while (i > 0) {
        i--;
        *UART_BASE = buf[i];
    }
    uart_print(" commands this session.\n");
    uart_print("\nTo exit QEMU: Press Ctrl+A then X\n");
    uart_print("To restart OS: Run 'make run' again\n");
    
    // Instead of halting, just loop and wait for QEMU exit
    while(1) {
        // Wait for user to press Ctrl+A X
    }
}
