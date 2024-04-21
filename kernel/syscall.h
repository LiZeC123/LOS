#pragma once

#include "stdint.h"
enum SYSCALL_NR { // 用来存放子功能号
  SYS_GETPID,
  SYS_WRITE
};
uint32_t getpid(void);
uint32_t write(char *str);
