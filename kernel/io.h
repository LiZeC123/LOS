#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
    __asm__ __volatile__ ("cld; rep outsw": "+S"(addr), "+c"(word_cnt): "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__ ("inb %w1, %b0" : "=a"(data) : "Nd" (port));
    return data;
}

static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {
    __asm__ __volatile__ ("cld; rep insw": "+D"(addr), "+c"(word_cnt): "d"(port): "memory");
}