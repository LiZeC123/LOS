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
#include "shell.h"
#include "stdio.h"
#include "string.h"
#include "syscall-init.h"
#include "thread.h"
#include "time.h"
#include "tss.h"

int prog_a_pid = 0, prog_b_pid = 0;

void init_all() {
  idt_init();
  time_init();
  console_init();
  mem_init();
  thread_init();
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
    while (1) {
    }

  } else {
    // 子进程
    my_shell();
  }
  while (1) {
  }

  // PANIC("init: should not be here");
}

int main() {
  put_str("Loading....\n");

  // 初始化所有的模块
  init_all();

  /*************    写入应用程序    *************/
  // extern IdeChannel channels[2]; // 有两个ide通道

  // uint32_t file_size = 2928;
  // uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
  // struct disk *sda = &channels[0].devices[0];
  // void *prog_buf = sys_malloc(file_size);
  // ide_read(sda, 300, prog_buf, sec_cnt);
  // int32_t fd = sys_open("/prog_no_arg", O_CREAT | O_RDWR);
  // if (fd != -1) {
  //   if (sys_write(fd, prog_buf, file_size) == -1) {
  //     printk("file write error!\n");
  //     while (1)
  //       ;
  //   }
  // }
  /*************    写入应用程序结束   *************/

 
  // cls_screen();

  // 准备就绪再开启中断, 允许线程被调度
  intr_enable();

  while (1)
    ;

  return 0;
}
