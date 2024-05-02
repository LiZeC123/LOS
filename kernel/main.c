#include "console.h"
#include "debug.h"
#include "func.h"
#include "ide.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "syscall-init.h"
#include "loscall.h"
#include "thread.h"
#include "time.h"
#include "tss.h"
#include "fs.h"

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
  ide_init();
  filesys_init();
}

/* 在线程中运行的函数 */
void k_thread_a(void *arg) {
  UNUSED(arg);
  void *addr1 = sys_malloc(256);
  void *addr2 = sys_malloc(255);
  void *addr3 = sys_malloc(254);
  console_put_str(" thread_a malloc addr:0x");
  console_put_int((int)addr1);
  console_put_char(',');
  console_put_int((int)addr2);
  console_put_char(',');
  console_put_int((int)addr3);
  console_put_char('\n');

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  sys_free(addr1);
  sys_free(addr2);
  sys_free(addr3);
  while (1)
    ;
}

/* 在线程中运行的函数 */
void k_thread_b(void *arg) {
  UNUSED(arg);
  void *addr1 = sys_malloc(256);
  void *addr2 = sys_malloc(255);
  void *addr3 = sys_malloc(254);
  console_put_str(" thread_b malloc addr:0x");
  console_put_int((int)addr1);
  console_put_char(',');
  console_put_int((int)addr2);
  console_put_char(',');
  console_put_int((int)addr3);
  console_put_char('\n');

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  sys_free(addr1);
  sys_free(addr2);
  sys_free(addr3);
  while (1)
    ;
}

void u_prog_a(void) {
  void *addr1 = malloc(256);
  void *addr2 = malloc(255);
  void *addr3 = malloc(254);
  printf(" prog_a malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1, (int)addr2,
         (int)addr3);

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  free(addr1);
  free(addr2);
  free(addr3);
  while (1)
    ;
}

void u_prog_b(void) {
  void *addr1 = malloc(256);
  void *addr2 = malloc(255);
  void *addr3 = malloc(254);
  printf(" prog_b malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1, (int)addr2,
         (int)addr3);

  int cpu_delay = 100000;
  while (cpu_delay-- > 0)
    ;
  free(addr1);
  free(addr2);
  free(addr3);
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

  process_execute(u_prog_a, "u_prog_a");
  process_execute(u_prog_b, "u_prog_b");
  thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
  thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

  while (1)
    ;

  return 0;
}
