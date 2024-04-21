#include "console.h"
#include "debug.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "syscall-init.h"
#include "syscall.h"
#include "thread.h"
#include "time.h"
#include "tss.h"

int prog_a_pid = 0, prog_b_pid = 0;

void init_all() {
  put_str("init_all...\n");
  idt_init();
  time_init();
  mem_init();
  thread_init();
  console_init();
  keyboard_init();
  tss_init();
  syscall_init();
}


void k_thread_a(void *name) {
  int32_t cnt = 33;
  void* addr = sys_malloc(cnt);
  printf("%s: malloc(%d): addr: 0x%x\n",name, cnt, addr);
  while (1)
    ;
}

void k_thread_b(void *name) {
  int32_t cnt = 63;
  void* addr = sys_malloc(cnt);
  printf("%s: malloc(%d): addr: 0x%x\n",name, cnt, addr);
  while (1)
    ;
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  thread_start("K-Thread-01", 31, k_thread_a, "ThreadA");
  thread_start("K-Thread-02", 31, k_thread_b, "ThreadB");

  while (1)
    ;

  return 0;
}
