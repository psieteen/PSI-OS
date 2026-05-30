#ifndef UTILS_H
#define UTILS_H

int str_eq(const char *a, const char *b);
void to_lower(char *s);
char* trim(char *s);
int starts_with(const char *str, const char *prefix);
void *memset(void *s, int c, unsigned long n);

#endif