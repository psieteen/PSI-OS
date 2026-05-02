// ============================================
// YOUR CONVERSATIONAL OS v2.1
// Fixed: Exit behavior, better persistence explanation
// ============================================

#define UART_BASE ((volatile unsigned int*)0x09000000)
#define UART_FR ((volatile unsigned int*)0x09000018)
#define UART_FR_RXFE (1 << 4)

#define MAX_NAME_LEN 32
#define MAX_HISTORY 10
#define MAX_CMD_LEN 256

// In QEMU, RAM resets on restart. For real persistence later.
char user_name[MAX_NAME_LEN] = {0};
char cmd_history[MAX_HISTORY][MAX_CMD_LEN];
int history_count = 0;
int history_position = 0;
unsigned int tick_count = 0;

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
        else if (c == 0x1b) {
            while (*UART_FR & UART_FR_RXFE);
            c = *UART_BASE & 0xFF;
            while (*UART_FR & UART_FR_RXFE);
            c = *UART_BASE & 0xFF;
            
            if (c == 'A' && history_count > 0) {
                while (i > 0) {
                    uart_print("\b \b");
                    i--;
                }
                history_position = (history_position - 1 + history_count) % history_count;
                char *hist_cmd = cmd_history[history_position];
                while (*hist_cmd) {
                    buffer[i++] = *hist_cmd;
                    *UART_BASE = *hist_cmd++;
                }
            }
        }
        else if (c >= ' ' && c <= '~') {
            buffer[i++] = c;
            *UART_BASE = c;
        }
    }
    buffer[max_len - 1] = '\0';
}

void to_lower(char *str) {
    while (*str) {
        if (*str >= 'A' && *str <= 'Z') {
            *str = *str + 32;
        }
        str++;
    }
}

int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

int starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

char* skip_spaces(char *str) {
    while (*str == ' ') str++;
    return str;
}

void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        int i;
        for (i = 0; cmd[i] && i < MAX_CMD_LEN - 1; i++) {
            cmd_history[history_count][i] = cmd[i];
        }
        cmd_history[history_count][i] = '\0';
        history_count++;
    } else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            int j;
            for (j = 0; cmd_history[i][j]; j++) {
                cmd_history[i-1][j] = cmd_history[i][j];
            }
            cmd_history[i-1][j] = '\0';
        }
        int i;
        for (i = 0; cmd[i] && i < MAX_CMD_LEN - 1; i++) {
            cmd_history[MAX_HISTORY-1][i] = cmd[i];
        }
        cmd_history[MAX_HISTORY-1][i] = '\0';
    }
    history_position = history_count;
}

void handle_command(char *cmd) {
    cmd = skip_spaces(cmd);
    
    if (cmd[0] == '\0') {
        uart_print("Type something...\n");
        return;
    }
    
    add_to_history(cmd);
    to_lower(cmd);
    tick_count++;
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        uart_print("Namaste ");
        if (user_name[0] != '\0') {
            uart_print(user_name);
            uart_print("! ");
        }
        uart_print("Welcome to your OS!\n");
    }
    else if (str_eq(cmd, "help")) {
        uart_print("\n===== COMMANDS =====\n");
        uart_print("  hi, hello    - Greeting\n");
        uart_print("  name         - Show current name\n");
        uart_print("  set name X   - Set your name\n");
        uart_print("  time         - Show uptime ticks\n");
        uart_print("  history      - Show command history\n");
        uart_print("  clear        - Clear screen\n");
        uart_print("  exit         - Show exit info\n");
        uart_print("  help         - This message\n");
        uart_print("===================\n\n");
        uart_print("[Shadow AI] Tracking ");
        int temp = tick_count;
        char buf[10];
        int i = 0;
        if (temp == 0) buf[i++] = '0';
        else {
            while (temp > 0) {
                buf[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        while (i > 0) {
            i--;
            *UART_BASE = buf[i];
        }
        uart_print(" commands so far\n\n");
    }
    else if (str_eq(cmd, "name")) {
        if (user_name[0] != '\0') {
            uart_print("Your name is: ");
            uart_print(user_name);
            uart_print("\n");
        } else {
            uart_print("No name set. Type 'set name YOURNAME'\n");
        }
    }
    else if (starts_with(cmd, "set name ")) {
        char *name = cmd + 9;
        int i;
        for (i = 0; name[i] && i < MAX_NAME_LEN - 1; i++) {
            user_name[i] = name[i];
        }
        user_name[i] = '\0';
        uart_print("Nice to meet you, ");
        uart_print(user_name);
        uart_print("! (Name lasts until QEMU restarts)\n");
    }
    else if (str_eq(cmd, "time")) {
        uart_print("Commands executed: ");
        int temp = tick_count;
        char buf[10];
        int i = 0;
        if (temp == 0) buf[i++] = '0';
        else {
            while (temp > 0) {
                buf[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        while (i > 0) {
            i--;
            *UART_BASE = buf[i];
        }
        uart_print("\n");
    }
    else if (str_eq(cmd, "history")) {
        uart_print("\n===== COMMAND HISTORY =====\n");
        for (int i = 0; i < history_count; i++) {
            char num[4];
            int j = 0;
            int temp = i + 1;
            while (temp > 0) {
                num[j++] = '0' + (temp % 10);
                temp /= 10;
            }
            while (j > 0) {
                j--;
                *UART_BASE = num[j];
            }
            uart_print(": ");
            uart_print(cmd_history[i]);
            uart_print("\n");
        }
        if (history_count == 0) {
            uart_print("No commands yet.\n");
        }
        uart_print("==========================\n\n");
    }
    else if (str_eq(cmd, "clear")) {
        for (int i = 0; i < 50; i++) uart_print("\n");
    }
    else if (str_eq(cmd, "exit") || str_eq(cmd, "quit")) {
        uart_print("\n========================================\n");
        uart_print("Session Summary:\n");
        uart_print("  Commands run: ");
        int temp = tick_count;
        char buf[10];
        int i = 0;
        if (temp == 0) buf[i++] = '0';
        else {
            while (temp > 0) {
                buf[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        while (i > 0) {
            i--;
            *UART_BASE = buf[i];
        }
        uart_print("\n");
        uart_print("  History saved: ");
        temp = history_count;
        i = 0;
        if (temp == 0) buf[i++] = '0';
        else {
            while (temp > 0) {
                buf[i++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        while (i > 0) {
            i--;
            *UART_BASE = buf[i];
        }
        uart_print("\n");
        uart_print("========================================\n\n");
        uart_print("To restart OS: Run 'make run' again\n");
        uart_print("To exit QEMU: Press Ctrl+A then X\n\n");
        
        // Don't halt - just wait for QEMU exit
        while(1) {
            // Wait for user to press Ctrl+A X
        }
    }
    else {
        uart_print("I don't understand '");
        uart_print(cmd);
        uart_print("'. Type 'help'\n");
    }
}

void kernel_main(void) {
    char input[256];
    
    uart_print("\n\n");
    uart_print("============================================\n");
    uart_print("     YOUR CONVERSATIONAL OS v2.1\n");
    uart_print("     Shadow AI Foundation Active\n");
    uart_print("============================================\n\n");
    
    uart_print("Type 'help' to see commands.\n");
    uart_print("Try: set name YourName, hi, history, time\n");
    uart_print("Up Arrow shows previous commands!\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        handle_command(input);
    }
}
