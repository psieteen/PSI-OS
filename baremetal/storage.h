#ifndef STORAGE_H
#define STORAGE_H

void storage_init(void);
void storage_save_name(const char *name);
int storage_load_name(char *name, int max_len);
void storage_save_all(void);
void storage_load_all(void);

#endif