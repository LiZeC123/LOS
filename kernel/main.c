#include "interrupt.h"
#include "print.h"
#include "debug.h"


void init_all() {
  put_str("init_all\n");
  idt_init();
}

int main() {
  char str[] = "Hello From Kernel.\n";
  put_str(str);

  // 初始化所有的模块
  init_all();
  


  while (1) {
    // 空循环占据CPU, 以免程序退出执行到其他代码
  };

  return 0;
}