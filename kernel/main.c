#include "interrupt.h"
#include "time.h"
#include "memory.h"
#include "print.h"
#include "debug.h"


void init_all() {
  put_str("init_all...\n");
  idt_init();
  time_init();
  mem_init();
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();
  
  put_str("Hello From Kernel.\n");

  void* addr = get_kernel_pages(3);

  PRINTLINE("Get Kernel Page Start vaddr is ", (uint32_t)addr);

  addr = get_kernel_pages(3);

  PRINTLINE("Get Kernel Page Start vaddr is ", (uint32_t)addr);
  
  intr_enable();

  while (1) {
    // 空循环占据CPU, 以免程序退出执行到其他代码
  };

  return 0;
}