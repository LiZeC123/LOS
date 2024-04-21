#include "syscall.h"

void syscall_init(void);
uint32_t sys_getpid(void);
uint32_t sys_write(char *str);