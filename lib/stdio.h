#include <stddef.h>
#include <stdint.h>

#define va_list char *

uint32_t vsprintf(char *str, const char *format, va_list ap);
uint32_t sprintf(char *buf, const char *format, ...);
uint32_t printf(const char *format, ...);
void printk(const char *format, ...);