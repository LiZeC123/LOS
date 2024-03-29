#include "memory.h"
#include "debug.h"
#include "print.h"
#include "string.h"
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
VirtualAddr kernel_vaddr;


#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

static void print_pool(const Pool *pool, const char *name) {
  put_str(name);
  next_line();

  PRINTLINE("    Size: ", pool->size);
  PRINTLINE("    Phy_start: ", pool->phy_addr_start);
  PRINTLINE("    Bitmap_start:", (unsigned int)pool->pool_map.bits);
  PRINTLINE("    Bitmap_size:", pool->pool_map.btmp_bytes_len);
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

  kernel_vaddr.vaddr_map.btmp_bytes_len = kbm_length;
  kernel_vaddr.vaddr_map.bits =
      (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

  kernel_vaddr.vaddr_start = K_HEAP_START;
  bitmap_init(&kernel_vaddr.vaddr_map);

  put_str("  mem_pool_init done\n");
}

static void *vaddr_get(PoolType t, uint32_t pages) {
  int vaddr_start = 0, bit_idx_start = -1;
  uint32_t cnt = 0;
  if (PF_KERNEL == t) {
    bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_map, pages);
    if (bit_idx_start == -1) {
      return NULL;
    }

    while (cnt < pages) {
      bitmap_set(&kernel_vaddr.vaddr_map, bit_idx_start + cnt, 1);
      cnt++;
    }
    vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
  } else {
    PAINC("vaddr_get for user is not implemented.");
  }

  return (void *)vaddr_start;
}

uint32_t *pte_ptr(uint32_t vaddr) {
  // 按照虚拟地址的规则得到对应的页表项的虚拟地址
  // 1. 页目录表的最后一项指向自己, 因此虚拟地址高10bit全部为1,
  // 将页目录表视为一个页表
  // 2. vaddr的高10bit作为索引, 可以定位vaddr的页表在页目录中的位置
  // 3. vaddr的中间10bit作为索引, 可以定位vaddr的页表项在页表中的位置
  // 4. 由于第3步处理器不会自动转换为偏移地址, 因此手动乘以4
  return (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                      PTE_IDX(vaddr) * 4);
}

uint32_t *pde_ptr(uint32_t vaddr) {
  // 把页目录视为页表这件事重复两次, 然后根据vaddr得到偏移地址
  return (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);
}

static void *palloc(Pool *pool) {
  int bit_idx = bitmap_scan(&pool->pool_map, 1);
  if (bit_idx == -1) {
    return NULL;
  }

  bitmap_set(&pool->pool_map, bit_idx, 1);
  uint32_t page_phyaddr = (bit_idx * PG_SIZE) + pool->phy_addr_start;
  return (void *)page_phyaddr;
}

static void page_table_add(void *_vaddr, void *_page_phyaddr) {
  uint32_t vaddr = (uint32_t)_vaddr;
  uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
  uint32_t *pde = pde_ptr(vaddr);
  uint32_t *pte = pte_ptr(vaddr);

  if (*pde & 0x00000001) {
    ASSERT(!(*pte & 0x00000001));

    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
  } else {
    // 如果页目录项不存在, 则先构建目录项
    uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);

    *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

    // 页表清空, 以免残余数据导致地址解析错误
    memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);

    ASSERT(!(*pte & 0x00000001));

    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
  }
}

void *malloc_page(PoolType t, uint32_t pages) {
  // 不超过15M
  ASSERT(pages > 0 && pages < 3840);

  void *vaddr_start = vaddr_get(t, pages);
  if (vaddr_start == NULL) {
    return NULL;
  }

  uint32_t vaddr = (uint32_t)vaddr_start, cnt = pages;
  Pool *pool = t == PF_KERNEL ? &kernel_pool : &user_pool;

  // 虚拟地址是连续的, 但物理可以可以不连续
  while (cnt-- > 0) {
    void *page_phyaddr = palloc(pool);
    if (page_phyaddr == NULL) {
      return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    vaddr += PG_SIZE;
  }

  return vaddr_start;
}

void *get_kernel_pages(uint32_t pages) {
  void *vaddr = malloc_page(PF_KERNEL, pages);
  if (vaddr != NULL) {
    memset(vaddr, 0, pages * PG_SIZE);
  }
  return vaddr;
}

void mem_init() {
  put_str("mem_init start\n");
  uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
  mem_pool_init(mem_bytes_total);
  put_str("mem_init done\n");
}