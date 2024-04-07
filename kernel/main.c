#include "console.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "time.h"
#include "keyboard.h"
#include "ioqueue.h"

void init_all() {
  put_str("init_all...\n");
  idt_init();
  time_init();
  mem_init();
  thread_init();
  console_init();
  keyboard_init();
}

void k_thread_a(void *arg) {
  while (1) {
    IntrStatus oldStatus = intr_disable();
    if(!ioq_empty(&KeybdBuf)) {
      console_put_str(arg);
      char byte = ioq_getchar(&KeybdBuf);
      console_put_char(byte);
    }
    intr_set_status(oldStatus);
  }
}

void k_thread_b(void *arg) {
  while (1) {
    IntrStatus oldStatus = intr_disable();
    if(!ioq_empty(&KeybdBuf)) {
      console_put_str(arg);
      char byte = ioq_getchar(&KeybdBuf);
      console_put_char(byte);
    }
    intr_set_status(oldStatus);
  }
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  thread_start("Thread-01", 31, k_thread_a, "A_");
  thread_start("Thread-02", 31, k_thread_b, "B_ ");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();
  
  while (1);
  
  // while (1) {
  //   console_put_str("Main ");
  // };

  return 0;
}
