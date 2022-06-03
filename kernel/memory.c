// #pragma once

// typedef enum { PF_KERNEL = 1, PF_USER = 2 } PoolType

// #define PG_P_0 0
// #define PG_P_1 1
// #define PG_RW_R 0
// #define PG_RW_W 2
// #define PG_US_S 0
// #define PG_US_U 4

#include "memory.h"
#include "print.h"
#include <stdint.h>

#define PG_SIZE 4096

#define MEM_BITMAP_BASE 0xc009a000

#define K_HEAP_START 0xc0100000

typedef struct {
  BitMap pool_map;
  uint32_t phy_addr_start;
  uint32_t size;
} Pool;

Pool kernel_pool, user_pool;
VirtualAddr kernel_addr;

#define PRINT(STR, NUMBER)                                                     \
  put_str(STR);                                                                \
  print_number(NUMBER);                                                        \
  next_line();

static void print_pool(const Pool *pool, const char *name) {
  put_str(name);
  next_line();

  PRINT("    Size: ", pool->size);
  PRINT("    Phy_start: ", pool->phy_addr_start);
  PRINT("    Bitmap_start:", (unsigned int)pool->pool_map.bits);
  PRINT("    Bitmap_size:", pool->pool_map.btmp_bytes_len);
}

static void mem_pool_init(uint32_t all_mem) {
  put_str("  mem_pool_init start\n");
  uint32_t page_table_size = PG_SIZE * 256;

  uint32_t used_mem = page_table_size + 0x100000;
  uint32_t free_mem = all_mem - used_mem;
  uint16_t all_free_pages = free_mem / PG_SIZE;

  uint16_t kernel_free_pages = all_free_pages / 2;
  uint16_t user_free_pages = all_free_pages - kernel_free_pages;

  uint32_t kbm_length = kernel_free_pages / 8;
  uint32_t ubm_length = user_free_pages / 8;

  uint32_t kp_start = used_mem; // 跳过低端1M+已经分配的页表占据的空间
  uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

  kernel_pool.phy_addr_start = kp_start;
  kernel_pool.size = kernel_free_pages * PG_SIZE;
  kernel_pool.pool_map.btmp_bytes_len = kbm_length;
  kernel_pool.pool_map.bits = (void *)MEM_BITMAP_BASE;

  user_pool.phy_addr_start = up_start;
  user_pool.size = user_free_pages * PG_SIZE;
  user_pool.pool_map.btmp_bytes_len = ubm_length;
  user_pool.pool_map.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

  print_pool(&kernel_pool, "  Kernel Pool:");
  print_pool(&user_pool, "  User Pool:");

  bitmap_init(&kernel_pool.pool_map);
  bitmap_init(&user_pool.pool_map);

  kernel_addr.vaddr_map.btmp_bytes_len = kbm_length;
  kernel_addr.vaddr_map.bits =
      (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

  kernel_addr.vaddr_start = K_HEAP_START;
  bitmap_init(&kernel_addr.vaddr_map);

  put_str("  mem_pool_init done\n");
}

void mem_init() {
  put_str("mem_init start\n");
  uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
  mem_pool_init(mem_bytes_total);
  put_str("mem_init done\n");
}