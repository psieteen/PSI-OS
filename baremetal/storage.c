#include "storage.h"
#include "uart.h"

void storage_init(void) {
    uart_print("[Storage] Placeholder mode (no persistence)\n");
}

void storage_save_name(const char *name) {
    (void)name;  // Unused parameter
    // Do nothing
}

int storage_load_name(char *name, int max_len) {
    (void)name;
    (void)max_len;
    return 0;  // No name loaded
}

void storage_save_all(void) {
    // Do nothing
}

void storage_load_all(void) {
    // Do nothing
}
