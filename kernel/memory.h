#pragma once

#include "bitmap.h"
#include "list.h"
#include <stdint.h>

typedef struct {
  BitMap vaddr_map; // 记录虚拟地址分配情况
  uint32_t vaddr_start; // 记录虚拟地址起始位置, 虚拟地址视为无限大,
                        // 分配时根据起始位置与idx值计算实际的地址
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
void *get_a_page(PoolType type, uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);

// 内存块
typedef struct mem_block {
    ListElem free_elem;
} MemBlock;

// 内存块描述符
typedef struct mem_block_desc {
    uint32_t block_size; // 内存块大小
    uint32_t blocks_per_arena; // 本 arena 中可容纳此 mem_block 的数量
    List free_list; // 目前可用的 mem_block 链表
} MemBlockDesc;

#define DESC_CNT 7 // 内存块描述符个数

// 初始化内存块描述符
void block_desc_init(MemBlockDesc *desc_array);

// 内存分配的系统调用实现
void *sys_malloc(uint32_t size);