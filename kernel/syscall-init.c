#include "syscall-init.h"

#include "exec.h"
#include "fork.h"
#include "fs.h"
#include "func.h"
#include "losmemory.h"
#include "print.h"
#include "string.h"
#include "thread.h"
#include "wait_exit.h"

#define SYS_CALL_MAX 256 // 最大支持的系统子功能调用数
typedef void *syscall;
syscall syscall_table[SYS_CALL_MAX];

// 返回当前任务的 pid
uint32_t sys_getpid(void) { return running_thread()->pid; }

void sys_clear() { cls_screen(); }

void sys_putchar(char char_ascii) { put_char(char_ascii); }

void sys_do_user_test_call() {
  void *p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
  p = sys_malloc(512);
  printk("malloc: %x\n", p);
}

// 将内核所在硬盘从300扇区开始的, 指定大小的内容作为文件写入文件系统所在的硬盘
void sys_make_user_prog(char *file_name, uint32_t file_size) {
  extern IdeChannel channels[2]; // 有两个ide通道

  uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
  struct disk *sda = &channels[0].devices[0];
  void *prog_buf = sys_malloc(sec_cnt * 512);

  ide_read(sda, 300, prog_buf, sec_cnt);
  int32_t fd = sys_open(file_name, O_CREAT | O_RDWR);

  if (fd != -1) {
    if (sys_write(fd, prog_buf, file_size) == -1) {
      printk("file write error!\n");
    }
  }

  sys_close(fd);
  sys_free(prog_buf);
}

void sys_empty_call() { printk("Empty Sys Call\n"); }

// 初始化系统调用
void syscall_init(void) {
  put_str("syscall_init ... ");
  syscall_table[SYS_RESTART] = sys_empty_call;
  syscall_table[SYS_EXIT] = sys_exit;
  syscall_table[SYS_FORK] = sys_fork;
  syscall_table[SYS_READ] = sys_read;
  syscall_table[SYS_WRITE] = sys_write;
  syscall_table[SYS_OPEN] = sys_open;
  syscall_table[SYS_CLOSE] = sys_close;
  syscall_table[SYS_WAITPID] = sys_wait;
  syscall_table[SYS_CREAT] = sys_empty_call;
  syscall_table[SYS_LINK] = sys_empty_call;
  syscall_table[SYS_UNLINK] = sys_unlink;
  syscall_table[SYS_EXECVE] = sys_execv;

  syscall_table[SYS_LSEEK] = sys_lseek;
  syscall_table[SYS_GETPID] = sys_getpid;

  syscall_table[SYS_MOUNT] = sys_empty_call;
  syscall_table[SYS_UMOUNT] = sys_empty_call;

  syscall_table[SYS_MALLOC] = sys_malloc;
  syscall_table[SYS_FREE] = sys_free;
  syscall_table[SYS_PUTCHAR] = sys_putchar;
  syscall_table[SYS_CLEAR] = sys_clear;

  syscall_table[SYS_GETCWD] = sys_getcwd;
  syscall_table[SYS_CHDIR] = sys_chdir;

  syscall_table[SYS_MKDIR] = sys_mkdir;
  syscall_table[SYS_RMDIR] = sys_rmdir;

  syscall_table[SYS_OPENDIR] = sys_opendir;
  syscall_table[SYS_CLOSEDIR] = sys_closedir;
  syscall_table[SYS_READDIR] = sys_readdir;
  syscall_table[SYS_REWINDDIR] = sys_rewinddir;
  syscall_table[SYS_PS] = sys_ps;

  syscall_table[SYS_STAT] = sys_stat;

  syscall_table[SYS_TEST] = sys_do_user_test_call;
  syscall_table[SYS_MKPROG] = sys_make_user_prog;
  syscall_table[SYS_PP] = sys_print_partition_info;

  put_str("done\n");
}