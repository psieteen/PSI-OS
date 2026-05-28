#include "uart.h"
#include "timer.h"
#include "shadow_ai.h"
#include "shell.h"

void kernel_main(void) {
    uart_init();
    timer_init();
    shadow_ai_init();
    shell_run();
}
