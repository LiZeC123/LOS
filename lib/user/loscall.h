#pragma once

#include "stdint.h"
#include "thread.h"

enum SYSCALL_NR { // 用来存放子功能号
  SYS_RESTART,
  SYS_FORK,
  SYS_READ,
  SYS_WRITE,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_WAITPID,
  SYS_CREAT,
  SYS_LINK,
  SYS_UNLINK,
  SYS_EXECVE,

  SYS_GETPID = 20,
  SYS_MOUNT,
  SYS_UMOUNT,
  SYS_MALLOC,
  SYS_FREE,
  SYS_PUTCHAR,
  SYS_CLEAR,
};

pid_t fork();
int32_t read(int32_t fd, void *buf, uint32_t count);
uint32_t write(int32_t fd, const void *buf, uint32_t count);

uint32_t getpid(void);

void *malloc(uint32_t size);
void free(void *ptr);

void putchar(char char_ascii);
void clear();