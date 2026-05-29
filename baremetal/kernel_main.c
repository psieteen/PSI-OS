#include "uart.h"
#include "timer.h"
#include "shadow_ai.h"
#include "storage.h"
#include "shell.h"

void kernel_main(void) {
    timer_init();
    shadow_ai_init();
    storage_init();
    shell_run();
}