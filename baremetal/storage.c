#include "storage.h"
#include "uart.h"
#include "utils.h"

#define STORAGE_BASE ((volatile char*)0x80000)
#define STORAGE_SIZE (4096)

static int storage_ready = 0;

void storage_init(void) {
    storage_ready = 1;
    uart_print("[Storage] Ready (in-memory mode)\n");
}

void storage_save_name(const char *name) {
    if (!storage_ready) return;
    
    volatile char *ptr = STORAGE_BASE;
    int offset = 0;
    
    ptr[offset++] = 'n';
    ptr[offset++] = 'a';
    ptr[offset++] = 'm';
    ptr[offset++] = 'e';
    ptr[offset++] = '=';
    
    while (*name && offset < STORAGE_SIZE - 10) {
        ptr[offset++] = *name++;
    }
    ptr[offset++] = '\0';
    
    uart_print("[Storage] Name saved\n");
}

int storage_load_name(char *name, int max_len) {
    if (!storage_ready) return 0;
    
    volatile char *ptr = STORAGE_BASE;
    int offset = 0;
    
    while (offset < STORAGE_SIZE - 10) {
        if (ptr[offset] == 'n' && ptr[offset+1] == 'a' && 
            ptr[offset+2] == 'm' && ptr[offset+3] == 'e' && ptr[offset+4] == '=') {
            offset += 5;
            int i = 0;
            while (ptr[offset] && ptr[offset] != '\0' && i < max_len - 1) {
                name[i++] = ptr[offset++];
            }
            name[i] = '\0';
            return 1;
        }
        offset++;
    }
    return 0;
}

void storage_save_all(void) {
    uart_print("[Storage] Save complete\n");
}

void storage_load_all(void) {
    uart_print("[Storage] Load complete\n");
}