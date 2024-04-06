#include "print.h"

void put_str(const char* str) {
  while (*str != '\0') put_char(*str++);
}

void next_line() { put_char('\n'); }

void print_number(uint64_t num) {
  if(num == 0) {
    put_str("0x0");
  }

  char buf[19];

  // 从后向前逐一计算每一位的16进制表示
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

  // 从头部开始, 找到不为0的项目
  idx = 2;
  while (buf[idx++] == '0');
  
  // 在头部写入0x前缀
  idx -= 3;
  buf[idx+0] = '0';
  buf[idx+1] = 'x';
  
  // 字符串结束
  buf[18] = '\0';

  put_str(&buf[idx]);
}