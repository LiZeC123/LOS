#include "syscall-init.h"
#include "console.h"
#include "print.h"
#include "string.h"
#include "thread.h"

#define SYS_CALL_MAX 32 // 最大支持的系统子功能调用数
typedef void *syscall;
syscall syscall_table[SYS_CALL_MAX];

// 初始化系统调用
void syscall_init(void) {
  put_str("syscall_init start\n");
  syscall_table[SYS_GETPID] = sys_getpid;
  syscall_table[SYS_WRITE] = sys_write;
  put_str("syscall_init done\n");
}

// 返回当前任务的 pid
uint32_t sys_getpid(void) { return running_thread()->pid; }

// 打印字符串 str (未实现文件系统前的版本)
uint32_t sys_write(char *str) {
  console_put_str(str);
  return strlen(str);
}
