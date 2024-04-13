#include "console.h"
#include "debug.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "thread.h"
#include "time.h"
#include "tss.h"

int test_var_a = 0, test_var_b = 0;

void init_all() {
  put_str("init_all...\n");
  idt_init();
  time_init();
  mem_init();
  thread_init();
  console_init();
  keyboard_init();
  tss_init();
}

void k_thread_a(void *arg) {
  while (1) {
    console_put_str(arg);
    console_put_int(test_var_a);
  }
}

void k_thread_b(void *arg) {
  while (1) {
    console_put_str(arg);
    console_put_int(test_var_b);
  }
}

void u_prog_a() {
  while (1) {
    test_var_a++;
  }
}

void u_prog_b() {
  while (1) {
    test_var_b++;
  }
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  thread_start("K-Thread-01", 31, k_thread_a, "A_");
  thread_start("K-Thread-02", 31, k_thread_b, "B_ ");
  process_execute(u_prog_a, "user_prog_a");
  process_execute(u_prog_b, "user_prog_b");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  while (1)
    ;

  return 0;
}
