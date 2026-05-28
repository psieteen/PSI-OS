#include "uart.h"

#define UART_BASE ((volatile unsigned char*)0x09000000)
#define UART_FR   ((volatile unsigned int*)0x09000018)
#define RXFE      (1 << 4)

static int initialized = 0;

void uart_init(void) {
    initialized = 1;
}

void uart_print(const char *s) {
    while (*s) *UART_BASE = *s++;
}

void uart_print_num(int n) {
    if (n == 0) { uart_print("0"); return; }
    char buf[12];
    int i = 0;
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        i--;
        *UART_BASE = buf[i];
    }
}

char uart_getchar(void) {
    while (*UART_FR & RXFE);
    return *UART_BASE;
}

void uart_readline(char *buf, int max) {
    int i = 0;
    char c;
    while (i < max - 1) {
        c = uart_getchar();
        if (c == '\r') {
            buf[i] = '\0';
            uart_print("\n");
            return;
        } else if (c == '\b' || c == 0x7f) {
            if (i > 0) {
                i--;
                uart_print("\b \b");
            }
        } else if (c >= ' ' && c <= '~') {
            buf[i++] = c;
            *UART_BASE = c;
        }
    }
    buf[max-1] = '\0';
}
