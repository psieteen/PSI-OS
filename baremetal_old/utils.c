#include "utils.h"

int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

void to_lower(char *s) {
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') *s = *s + 32;
        s++;
    }
}

char* trim(char *s) {
    while (*s == ' ') s++;
    return s;
}
