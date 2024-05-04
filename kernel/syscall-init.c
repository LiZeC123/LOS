#include "syscall-init.h"

#include "exec.h"
#include "fork.h"
#include "fs.h"
#include "losmemory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

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

void sys_empty_call() { printk("Empty Sys Call\n"); }

// 初始化系统调用
void syscall_init(void) {
  put_str("syscall_init start\n");
  syscall_table[SYS_RESTART] = sys_empty_call;
  syscall_table[SYS_EXIT] = sys_empty_call;
  syscall_table[SYS_FORK] = sys_fork;
  syscall_table[SYS_READ] = sys_read;
  syscall_table[SYS_WRITE] = sys_write;
  syscall_table[SYS_OPEN] = sys_open;
  syscall_table[SYS_CLOSE] = sys_close;
  syscall_table[SYS_WAITPID] = sys_empty_call;
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

  put_str("syscall_init done\n");
}