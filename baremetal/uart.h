#ifndef UART_H
#define UART_H

void uart_print(const char *s);
void uart_print_num(int n);
void uart_print_char(char c);
char uart_getchar(void);
void uart_readline(char *buf, int max);

#endif