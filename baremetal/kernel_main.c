#include "uart.h"
#include "timer.h"
#include "shadow_ai.h"
// #include "storage.h"  // Temporarily disabled
#include "shell.h"

void kernel_main(void) {
    timer_init();
    shadow_ai_init();
    // storage_init();  // Temporarily disabled
    shell_run();
}
