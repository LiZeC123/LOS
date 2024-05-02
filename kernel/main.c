#include "console.h"
#include "debug.h"
#include "dir.h"
#include "fs.h"
#include "func.h"
#include "ide.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "loscall.h"
#include "losmemory.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "syscall-init.h"
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
  ide_init();
  filesys_init();
}

// initi进程
void init() {
  uint32_t ret_pid = fork();
  if (ret_pid) {
    printf("i am father, my pid is %d child pid is %d\n", getpid(), ret_pid);
    // int status;
    // int child_pid;
    // while (1) {
    //   child_pid = wait(&status);
    //   printf("I am init, I recieve a child pid is %d status is %d\n",
    //   child_pid,
    //          status);
    // }
  } else {
    printf("I am child, my pid is %d, ret pid is %d", getpid(), ret_pid);
    // my_shell();
  }
  while (1) {
  }

  // PANIC("init: should not be here");
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  put_str("Hello From Kernel.\n");

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  while (1)
    ;

  return 0;
}
