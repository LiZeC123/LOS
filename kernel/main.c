#include "print.h"

void put_str(const char* str, int len);

// 注意: 由于入口点是强制指定的地址, 因此main必须是这个文件中的第一个函数
// 从而在text段中main的代码才能处于开始位置
int main() {
  char str[] = "Hello From Kernel.";
  const int L = sizeof(str) / sizeof(char) - 1;
  put_str(str, L);
  next_line();

  for (int i = 0; i < 30; i++) {
    put_str("Line ", 5);
    put_char('1' + i % 10);
    next_line();
  }

  while (1) {
  };

  return 0;
}

void put_str(const char* str, int len) {
  for (int i = 0; i < len; i++) {
    put_char(str[i]);
  }
}

void next_line() { put_char('\n'); }