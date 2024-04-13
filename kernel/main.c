#include "console.h"
#include "debug.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
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

void k_thread_a(void *arg) {
  char *para = arg;
  console_put_str("  thread_a_pid:");
  console_put_int(sys_getpid());
  console_put_str("\n");
  console_put_str("  prog_a_pid:");
  console_put_int(prog_a_pid);
  console_put_str("\n");
  while (1)
    ;
}

void k_thread_b(void *arg) {
  char *para = arg;
  console_put_str("  thread_b_pid:");
  console_put_int(sys_getpid());
  console_put_str("\n");
  console_put_str("  prog_b_pid:");
  console_put_int(prog_b_pid);
  console_put_str("\n");
  while (1)
    ;
}

void u_prog_a() {
  prog_a_pid = getpid();
  while (1) {
  }
}

void u_prog_b() {
  prog_b_pid = getpid();
  while (1) {
  }
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  process_execute(u_prog_a, "user_prog_a");
  process_execute(u_prog_b, "user_prog_b");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  thread_start("K-Thread-01", 31, k_thread_a, "A_");
  thread_start("K-Thread-02", 31, k_thread_b, "B_ ");
  console_put_str("Main pid:");
  console_put_int(sys_getpid());

  while (1)
    ;

  return 0;
}
