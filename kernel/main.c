#include "console.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "time.h"

void init_all() {
  put_str("init_all...\n");
  idt_init();
  time_init();
  mem_init();
  thread_init();
  console_init();
}

void k_thread_a(void *arg) {
  char *para = arg;
  while (1) {
    console_put_str(para);
  }
}

void k_thread_b(void *arg) {
  char *para = arg;
  while (1) {
    console_put_str(para);
  }
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  thread_start("Thread-01", 31, k_thread_a, "argA ");
  thread_start("Thread-02", 8, k_thread_b, "argB ");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  while (1) {
    console_put_str("Main ");
  };

  return 0;
}
