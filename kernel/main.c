#include "interrupt.h"
#include "print.h"


void init_all() {
  put_str("init_all\n");
  idt_init();
}

int main() {
  char str[] = "Hello From Kernel.\n";
  put_str(str);

  // 初始化所有的模块
  init_all();
  
  // 临时开中断
  __asm__ __volatile__("sti");


  while (1) {
    // 空循环占据CPU, 以免程序退出执行到其他代码
  };

  return 0;
}