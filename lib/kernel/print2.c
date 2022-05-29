#include "print.h"

void put_str(const char* str) {
  while (*str != '\0') put_char(*str++);
}

void next_line() { put_char('\n'); }

void print_number(uint64_t num) {
  char buf[19];

  int idx = 17;
  while (idx >= 0) {
    int v = num % 16;
    char c = '0';
    if (v < 10) {
      c = '0' + v;
    } else {
      c = 'a' + (v - 10);
    }

    buf[idx--] = c;
    num /= 16;
  }

  buf[0] = '0';
  buf[1] = 'x';
  buf[18] = '\0';

  put_str(buf);
}