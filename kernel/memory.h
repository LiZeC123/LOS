#pragma once

#include "bitmap.h"
#include <stdint.h>

typedef struct {
  BitMap vaddr_map; // 记录虚拟地址分配情况
  uint32_t vaddr_start; // 记录虚拟地址起始位置, 虚拟地址视为无限大,
                        // 分配时调整起始位置
} VirtualAddr;

typedef enum { PF_KERNEL = 1, PF_USER = 2 } PoolType;

#define PG_P_0 0
#define PG_P_1 1
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0
#define PG_US_U 4

void mem_init();
void *get_kernel_pages(uint32_t pages);