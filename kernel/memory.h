#pragma once

#include "bitmap.h"
#include <stdint.h>

typedef struct {
  BitMap vaddr_map;
  uint32_t vaddr_start;
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