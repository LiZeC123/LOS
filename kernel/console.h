#pragma once

#include <stdint.h>

void console_init();

void console_put_str(const char *str);

void console_put_char(uint8_t c);

void console_put_int(uint32_t n);