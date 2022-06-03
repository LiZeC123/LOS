#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "debug.h"


void init_all() {
  put_str("init_all...\n");
  idt_init();
  mem_init();
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();
  
  put_str("Hello From Kernel.\n");

  while (1) {
    // 空循环占据CPU, 以免程序退出执行到其他代码
  };

  return 0;
}