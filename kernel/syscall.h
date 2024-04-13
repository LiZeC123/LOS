#pragma once

#include "stdint.h"
enum SYSCALL_NR { // 用来存放子功能号
  SYS_GETPID
};
uint32_t getpid(void);

