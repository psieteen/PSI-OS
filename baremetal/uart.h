#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_print(const char *s);
void uart_print_num(int n);
char uart_getchar(void);
void uart_readline(char *buf, int max);

#endif
