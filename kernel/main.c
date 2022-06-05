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

  thread_start("k_thread_a", 31, k_thread_a, "OOOO ");
  thread_start("k_thread_b", 8, k_thread_b, "XXXX ");

  while (1) {
    console_put_str("Main ");
  };

  return 0;
}
