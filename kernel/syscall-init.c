#include "syscall-init.h"
#include "console.h"
#include "fork.h"
#include "fs.h"
#include "losmemory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

#define SYS_CALL_MAX 32 // 最大支持的系统子功能调用数
typedef void *syscall;
syscall syscall_table[SYS_CALL_MAX];

// 初始化系统调用
void syscall_init(void) {
  put_str("syscall_init start\n");
  syscall_table[SYS_FORK] = sys_fork;
  syscall_table[SYS_GETPID] = sys_getpid;
  syscall_table[SYS_WRITE] = sys_write;
  syscall_table[SYS_MALLOC] = sys_malloc;
  syscall_table[SYS_FREE] = sys_free;
  put_str("syscall_init done\n");
}

// 返回当前任务的 pid
uint32_t sys_getpid(void) { return running_thread()->pid; }
