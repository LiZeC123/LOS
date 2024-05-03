#include "loscall.h"

/* 无参数的系统调用 */
#define _syscall0(NUMBER)                                                      \
  ({                                                                           \
    int retval;                                                                \
    __asm__ __volatile__("int $0x80" : "=a"(retval) : "a"(NUMBER) : "memory"); \
    retval;                                                                    \
  })

/* 一个参数的系统调用 */
#define _syscall1(NUMBER, ARG1)                                                \
  ({                                                                           \
    int retval;                                                                \
    __asm__ __volatile__("int $0x80"                                           \
                         : "=a"(retval)                                        \
                         : "a"(NUMBER), "b"(ARG1)                              \
                         : "memory");                                          \
    retval;                                                                    \
  })

/* 两个参数的系统调用 */
#define _syscall2(NUMBER, ARG1, ARG2)                                          \
  ({                                                                           \
    int retval;                                                                \
    __asm__ __volatile__("int $0x80"                                           \
                         : "=a"(retval)                                        \
                         : "a"(NUMBER), "b"(ARG1), "c"(ARG2)                   \
                         : "memory");                                          \
    retval;                                                                    \
  })

/* 三个参数的系统调用 */
#define _syscall3(NUMBER, ARG1, ARG2, ARG3)                                    \
  ({                                                                           \
    int retval;                                                                \
    __asm__ __volatile__("int $0x80"                                           \
                         : "=a"(retval)                                        \
                         : "a"(NUMBER), "b"(ARG1), "c"(ARG2), "d"(ARG3)        \
                         : "memory");                                          \
    retval;                                                                    \
  })

// 复制当前进程, 分裂为两个进程
pid_t fork() { return _syscall0(SYS_FORK); }

// 从文件描述符fd读取count字节到buf
int32_t read(int32_t fd, void *buf, uint32_t count) {
  return _syscall3(SYS_READ, fd, buf, count);
}

// 从buf输出count字节到文件描述符fd对应的文件
uint32_t write(int32_t fd, const void *buf, uint32_t count) {
  return _syscall3(SYS_WRITE, fd, buf, count);
}

// 以指定的模式打开文件
int32_t open(char *pathname, uint8_t flag) {
  return _syscall2(SYS_OPEN, pathname, flag);
}

// 关闭文件
int32_t close(int32_t fd) { return _syscall1(SYS_CLOSE, fd); }

// 删除文件
int32_t unlink(const char *pathname) { return _syscall1(SYS_UNLINK, pathname); }

// 设置文件偏移量
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence) {
  return _syscall3(SYS_LSEEK, fd, offset, whence);
}

// 返回当前任务pid
uint32_t getpid() { return _syscall0(SYS_GETPID); }

// 申请 size 字节大小的内存, 并返回结果
void *malloc(uint32_t size) { return (void *)_syscall1(SYS_MALLOC, size); }

// 释放 ptr 指向的内存
void free(void *ptr) { _syscall1(SYS_FREE, ptr); }

// 向屏幕输出一个字符(含控制字符)
void putchar(char char_ascii) { _syscall1(SYS_PUTCHAR, char_ascii); }

// 清空屏幕
void clear() { _syscall0(SYS_CLEAR); }

// 获取当前工作目录
char *getcwd(char *buf, uint32_t size) {
  return (char *)_syscall2(SYS_GETCWD, buf, size);
}

// 改变工作目录为path
int32_t chdir(const char *path) { return _syscall1(SYS_CHDIR, path); }

// 创建目录pathname
int32_t mkdir(const char *pathname) { return _syscall1(SYS_MKDIR, pathname); }

// 删除目录pathname
int32_t rmdir(const char *pathname) { return _syscall1(SYS_RMDIR, pathname); }

// 打开目录name
struct dir *opendir(const char *name) {
  return (struct dir *)_syscall1(SYS_OPENDIR, name);
}

// 关闭目录dir
int32_t closedir(struct dir *dir) { return _syscall1(SYS_CLOSEDIR, dir); }

// 读取目录dir
struct dir_entry *readdir(struct dir *dir) {
  return (struct dir_entry *)_syscall1(SYS_READDIR, dir);
}

// 回归目录指针
void rewinddir(struct dir *dir) { _syscall1(SYS_REWINDDIR, dir); }

// 显示任务列表 */
void ps(void) { _syscall0(SYS_PS); }

// 获取path属性到buf中
int32_t stat(const char *path, struct stat *buf) {
  return _syscall2(SYS_STAT, path, buf);
}

void do_user_test_call() { _syscall0(SYS_TEST); }