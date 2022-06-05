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

  intr_enable();

  thread_start("a", 31, k_thread_a, "aaa ");
  thread_start("b", 8, k_thread_a, "bbb ");
  thread_start("c", 14, k_thread_a, "ccc ");
  thread_start("d", 21, k_thread_a, "ddd ");
  thread_start("e", 17, k_thread_a, "eee ");
  thread_start("f", 5, k_thread_a, "fff ");

  while (1) {
    console_put_str("Main ");
  };

  return 0;
}
