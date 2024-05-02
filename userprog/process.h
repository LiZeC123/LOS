#pragma once
#include "thread.h"

// 当前内存布局下 0xC000_0000 开始的1GB空间为内核空间
// 因此从这个位置申请的1页空间为用户可使用的最高地址空间
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)

// 用户进程的虚拟地址起始位置
// 通常Linux将该位置作为程序的起始位置
#define USER_VADDR_START 0x8048000

// 用户进程默认优先级
#define DEFAULT_PRIO 31

void process_execute(void *filename, char *name);
void process_active(TaskStruct *p_thread);
void page_dir_activate(TaskStruct *p_thread);
uint32_t *create_page_dir();