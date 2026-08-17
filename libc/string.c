#include <libc/string.h>

void* memset(void* dest, int char_val, size_t count){
    uint8_t* ptr = (uint8_t*)dest;
    while (count--){
        *ptr++ = (uint8_t)char_val;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count){
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (count--){
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t size){
    const uint8_t *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < size; i++){
        if (p1[i] != p2[i]) return p1[i] < p2[i] ? -1 : 1;
    }
    return 0;
}

int strlen(const char *s){
    size_t len = 0;
    while (s[len] != '\0') len++;
    return len;
}