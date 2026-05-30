#include "uart.h"
#include "utils.h"
#include "shadow_ai.h"

// Global state
char username[32] = {0};
char last_command[32] = {0};

void process_command(char *cmd) {
    cmd = trim(cmd);
    if (cmd[0] == '\0') {
        uart_print("Type something...\n");
        return;
    }
    
    // Learn sequence (from last command to this command)
    if (last_command[0] != '\0') {
        learn_sequence(last_command, cmd);
    }
    
    // Learn time pattern
    learn_pattern(cmd);
    
    // Save as last command
    int j;
    for (j = 0; cmd[j] && j < 31; j++) last_command[j] = cmd[j];
    last_command[j] = '\0';
    
    // Process command
    to_lower(cmd);
    
    if (str_eq(cmd, "hi") || str_eq(cmd, "hello")) {
        uart_print("Namaste ");
        if (username[0]) uart_print(username);
        uart_print("!\n");
    }
    else if (str_eq(cmd, "help")) {
        uart_print("\n===== COMMANDS =====\n");
        uart_print("  hi        - Greeting\n");
        uart_print("  name      - Show name\n");
        uart_print("  set name X- Set name\n");
        uart_print("  patterns  - Show patterns\n");
        uart_print("  sequences - Show sequences\n");
        uart_print("  predict   - Predict next\n");
        uart_print("  clear     - Clear screen\n");
        uart_print("  exit      - Exit\n");
        uart_print("===================\n\n");
    }
    else if (str_eq(cmd, "name")) {
        if (username[0]) { uart_print("Name: "); uart_print(username); uart_print("\n"); }
        else uart_print("No name set.\n");
    }
    else if (cmd[0]=='s' && cmd[1]=='e' && cmd[2]=='t' && cmd[3]==' ' &&
             cmd[4]=='n' && cmd[5]=='a' && cmd[6]=='m' && cmd[7]=='e' && cmd[8]==' ') {
        char *name = cmd + 9;
        int i;
        for (i = 0; name[i] && i < 31; i++) username[i] = name[i];
        username[i] = '\0';
        uart_print("Hello "); uart_print(username); uart_print("!\n");
    }
    else if (str_eq(cmd, "patterns")) {
        show_patterns();
    }
    else if (str_eq(cmd, "sequences")) {
        show_sequences();
    }
    else if (str_eq(cmd, "predict")) {
        predict(last_command);
    }
    else if (str_eq(cmd, "clear")) {
        for (int i = 0; i < 30; i++) uart_print("\n");
    }
    else if (str_eq(cmd, "exit")) {
        uart_print("Goodbye!\n");
        while(1);
    }
    else {
        uart_print("Unknown: '"); uart_print(cmd); uart_print("'\n");
    }
}

void kernel_main(void) {
    char input[128];
    
    shadow_ai_init();
    
    uart_print("\n\n");
    uart_print("========================================\n");
    uart_print("     CLEAN OS v10.0\n");
    uart_print("     Modular, Maintainable\n");
    uart_print("========================================\n\n");
    uart_print("Type 'help'\n\n");
    
    while (1) {
        uart_print(">> ");
        uart_readline(input, sizeof(input));
        process_command(input);
    }
}
