#include "losmemory.h"
#include "debug.h"
#include "func.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "print.h"
#include "string.h"
#include "sync.h"
#include "thread.h"
#include <stdint.h>

#define PG_SIZE 4096

// 位图结构的起始位置
// 从此处开始的4页内存均可使用
// 0xC009_E000开始的1页为内核主线程的PCB块
#define MEM_BITMAP_BASE 0xc009a000

#define K_HEAP_START 0xc0100000

typedef struct {
  BitMap pool_map;
  uint32_t phy_addr_start;
  uint32_t size; // 内存可用字节数
  Lock lock;
} Pool;

Pool kernel_pool,
    user_pool; // 定义两个内存池, 管理内核空间和用户空间的物理地址分配
VirtualAddr kernel_vaddr; // 管理内核虚拟地址分配

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

static void print_pool(const Pool *pool, const char *name) {
  printk("%s: Size: %dKB Phy_start: 0x%x\n  Bitmap_start: 0x%x Bitmap_size: %d "
         "Byte\n",
         name, pool->size / 1024, pool->phy_addr_start,
         (unsigned int)pool->pool_map.bits, pool->pool_map.btmp_bytes_len);
}

static void mem_pool_init(uint32_t all_mem) {
  // 计算当前页表已经使用的空间
  // 1个页目录表(1页)+页目录第0项和第0x301项指向的页表(1页)+内核空间245项页目录项指向的页表(254页)
  // 因此当前页表数据正好使用了256页, 即1M内存空间
  uint32_t page_table_size = PG_SIZE * 256;
  // 页表相关数据结构占据的空间 + 内核代码的 1M 空间 = 当前已经使用的内存空间
  uint32_t used_mem = page_table_size + 0x100000;
  uint32_t free_mem = all_mem - used_mem;
  uint16_t all_free_pages = free_mem / PG_SIZE;

  uint16_t kernel_free_pages = all_free_pages / 2;
  uint16_t user_free_pages = all_free_pages - kernel_free_pages;

  uint32_t kbm_length = kernel_free_pages / 8;
  uint32_t ubm_length = user_free_pages / 8;

  uint32_t kp_start = used_mem; // 跳过低端已经分配的空间(内核代码+页表)
  uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

  kernel_pool.phy_addr_start = kp_start;
  kernel_pool.size = kernel_free_pages * PG_SIZE;
  kernel_pool.pool_map.btmp_bytes_len = kbm_length;
  kernel_pool.pool_map.bits = (void *)MEM_BITMAP_BASE;

  user_pool.phy_addr_start = up_start;
  user_pool.size = user_free_pages * PG_SIZE;
  user_pool.pool_map.btmp_bytes_len = ubm_length;
  user_pool.pool_map.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

  print_pool(&kernel_pool, "Kernel Pool");
  print_pool(&user_pool, "User Pool");

  bitmap_init(&kernel_pool.pool_map);
  bitmap_init(&user_pool.pool_map);

  kernel_vaddr.vaddr_map.btmp_bytes_len = kbm_length;
  kernel_vaddr.vaddr_map.bits =
      (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

  kernel_vaddr.vaddr_start = K_HEAP_START;
  bitmap_init(&kernel_vaddr.vaddr_map);
}

// 从虚拟内存池中申请内存页
static void *vaddr_get(PoolType t, uint32_t pages) {
  int vaddr_start = 0, bit_idx_start = -1;

  if (PF_KERNEL == t) {
    bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_map, pages);
    if (bit_idx_start == -1) {
      return NULL;
    }

    for (uint32_t cnt = 0; cnt < pages; cnt++) {
      bitmap_set(&kernel_vaddr.vaddr_map, bit_idx_start + cnt, 1);
    }
    vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
  } else {
    TaskStruct *cur = running_thread();
    bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_map, pages);
    if (bit_idx_start == -1) {
      return NULL;
    }

    for (uint32_t cnt = 0; cnt < pages; cnt++) {
      bitmap_set(&cur->userprog_vaddr.vaddr_map, bit_idx_start + cnt, 1);
    }
    vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;

    // 用户的3级栈初始位于3GB以下的最后1页, 因此虚拟地址不应该分配到此位置
    ASSERT((uint32_t)vaddr_start < (0xC0000000 - PG_SIZE));
  }

  return (void *)vaddr_start;
}

uint32_t *pte_ptr(uint32_t vaddr) {
  // 按照虚拟地址的规则得到对应的页表项的虚拟地址
  // 先将页目录项视为页表项进行转换,
  // 使得虚拟地址对应的内存页正好是页表项所在的内存页
  // 在根据页表项索引计算出页表项的偏移地址
  return (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                      PTE_IDX(vaddr) * 4);
}

uint32_t *pde_ptr(uint32_t vaddr) {
  // 把页目录视为页表这件事重复两次,
  // 从而使虚拟地址对应的内存页正好是页目录项所在的内存页
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

// 将物理地址 pg_phy_addr 回收到物理内存池
void pfree(uint32_t pg_phy_addr) {
  Pool *mem_pool;
  uint32_t bit_idx = 0;

  // 内核使用低15M的内存, 因此如果物理地址大于用户的起始物理地址,
  // 则该内存属于用户物理内存池
  if (pg_phy_addr >= user_pool.phy_addr_start) {
    mem_pool = &user_pool;
    bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
  } else {
    mem_pool = &kernel_pool;
    bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
  }

  bitmap_set(&mem_pool->pool_map, bit_idx, 0);
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

// 去掉页表中虚拟地址 vaddr 的映射, 只去掉 vaddr 对应的 pte
static void page_table_pte_remove(uint32_t vaddr) {
  uint32_t *pte = pte_ptr(vaddr);
  *pte &= ~PG_P_1; // 将页表项 pte 的 P 位置 0, 向CPU表示该页已经失效
  __asm__ __volatile__("invlpg %0" : : "m"(vaddr) : "memory"); // 更新 tlb
}

// 在虚拟地址池中释放以 vaddr 起始的连续 pg_cnt 个虚拟页地址
static void vaddr_remove(PoolType pf, void *_vaddr, uint32_t pg_cnt) {
  uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr;

  if (pf == PF_KERNEL) {
    bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
    for (uint32_t cnt = 0; cnt < pg_cnt; cnt++) {
      bitmap_set(&kernel_vaddr.vaddr_map, bit_idx_start + cnt, 0);
    }
  } else {
    TaskStruct *cur_thread = running_thread();
    bit_idx_start = (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PG_SIZE;
    for (uint32_t cnt = 0; cnt < pg_cnt; cnt++) {
      bitmap_set(&cur_thread->userprog_vaddr.vaddr_map, bit_idx_start + cnt, 0);
    }
  }
}

// 申请分配物理页, 返回该物理页对应的虚拟地址
void *malloc_page(PoolType t, uint32_t pages) {
  // 不超过15M
  ASSERT(pages > 0 && pages < 3840);

  // 申请虚拟地址, 对于内核线程共享虚拟地址
  // 对于用户进程, 从用户进程自带的虚拟地址管理器中申请虚拟地址
  void *vaddr_start = vaddr_get(t, pages);
  if (vaddr_start == NULL) {
    return NULL;
  }

  uint32_t vaddr = (uint32_t)vaddr_start, cnt = pages;
  Pool *pool = (t == PF_KERNEL ? &kernel_pool : &user_pool);

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

// 释放以虚拟地址 vaddr 为起始的 cnt 个物理页框
void mfree_page(PoolType pf, void *_vaddr, uint32_t pg_cnt) {
  uint32_t pg_phy_addr;
  uint32_t vaddr = (int32_t)_vaddr, page_cnt = 0;
  ASSERT((pg_cnt >= 1) && (vaddr % PG_SIZE) == 0);

  // 获取对应的物理地址
  pg_phy_addr = addr_v2p(vaddr);
  // 确保待释放的物理内存在低端 1MB+1KB 大小的页目录 + 1KB 大小的页表地址外
  ASSERT((pg_phy_addr % PG_SIZE) == 0 && pg_phy_addr >= 0x102000);

  // 判断 pg_phy_addr 属于用户物理内存池还是内核物理内存池
  if (pg_phy_addr >= user_pool.phy_addr_start) {
    vaddr -= PG_SIZE;
    while (page_cnt < pg_cnt) {
      vaddr += PG_SIZE;
      pg_phy_addr = addr_v2p(vaddr);
      // 确保物理地址属于用户物理地址池
      ASSERT((pg_phy_addr % PG_SIZE) == 0 &&
             pg_phy_addr >= user_pool.phy_addr_start);

      pfree(pg_phy_addr);
      page_table_pte_remove(vaddr);

      page_cnt++;
    }
    // 清空虚拟地址的位图中的相应位
    vaddr_remove(pf, _vaddr, pg_cnt);
  } else { // 位于内核物理内存池
    vaddr -= PG_SIZE;
    while (page_cnt < pg_cnt) {
      vaddr += PG_SIZE;
      pg_phy_addr = addr_v2p(vaddr);
      // 确保待释放的物理内存只属于内核物理地址池
      ASSERT((pg_phy_addr % PG_SIZE) == 0 &&
             pg_phy_addr >= kernel_pool.phy_addr_start &&
             pg_phy_addr < user_pool.phy_addr_start);

      pfree(pg_phy_addr);
      page_table_pte_remove(vaddr);

      page_cnt++;
    }
    // 清空虚拟地址的位图中的相应位
    vaddr_remove(pf, _vaddr, pg_cnt);
  }
}

// 在内核空间申请pages页内存
void *get_kernel_pages(uint32_t pages) {
  void *vaddr = malloc_page(PF_KERNEL, pages);
  if (vaddr != NULL) {
    memset(vaddr, 0, pages * PG_SIZE);
  }
  return vaddr;
}

// 在用户空间申请pages页内存
void *get_user_pages(uint32_t pages) {
  lock_acquire(&user_pool.lock);
  void *vaddr = malloc_page(PF_USER, pages);
  memset(vaddr, 0, pages * PG_SIZE);
  lock_release(&user_pool.lock);
  return vaddr;
}

// 申请一页内存空间, 将指定的虚拟地址映射到该内存页
// 与malloc相比, 可自行指定虚拟内存的映射关系
void *get_a_page(PoolType type, uint32_t vaddr) {
  Pool *pool = (type == PF_KERNEL ? &kernel_pool : &user_pool);
  lock_acquire(&pool->lock);

  TaskStruct *cur = running_thread();
  int32_t bit_idx = -1;

  if (cur->pgdir != NULL && type == PF_USER) {
    // 如果是用户进程并且申请用户内存
    bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
    ASSERT(bit_idx > 0);
    bitmap_set(&cur->userprog_vaddr.vaddr_map, bit_idx, 1);
  } else if (cur->pgdir == NULL && type == PF_KERNEL) {
    // 如果是内核线程并且申请内核内存
    bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
    ASSERT(bit_idx > 0);
    bitmap_set(&kernel_vaddr.vaddr_map, bit_idx, 1);
  } else {
    // 不允许交叉申请内存, 例如用户申请内核内存或内核申请用户内存
    PANIC("get_a_page: not allow kernel alloc usersapce or user alloc "
          "kernelspace.");
  }

  void *page_phyaddr = palloc(pool);
  if (page_phyaddr == NULL) {
    return NULL;
  }

  page_table_add((void *)vaddr, page_phyaddr);
  lock_release(&pool->lock);
  return (void *)vaddr;
}

// 虚拟地址转换为物理地址
uint32_t addr_v2p(uint32_t vaddr) {
  // pte指向虚拟地址对应的页表项, *pte是该项的具体数据
  // 其中高20位是虚拟地址页对应的物理地址页, 与虚拟地址低12位组合
  // 即为最终的物理地址
  uint32_t *pte = pte_ptr(vaddr);
  return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

// ======================== Arena 内存分配 =========================

// 内存仓库 arena 元信息
typedef struct arena {
  MemBlockDesc *desc;
  // large 为 true 时, cnt 表示的是页框数
  // 否则 cnt 表示空闲 mem_block 数量
  uint32_t cnt;
  bool large;
} Arena;

// 内核内存块描述符数组
MemBlockDesc k_block_descs[DESC_CNT];

void block_desc_init(MemBlockDesc *desc_array) {
  uint16_t desc_idx, block_size = 16;

  // 初始化每个 mem_block_desc 描述符
  for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
    desc_array[desc_idx].block_size = block_size;

    // 初始化 arena 中的内存块数量
    desc_array[desc_idx].blocks_per_arena =
        (PG_SIZE - sizeof(struct arena)) / block_size;
    list_init(&desc_array[desc_idx].free_list);
    block_size *= 2; // 更新为下一个规格内存块
  }
}

// 返回 arena 中第 idx 个内存块的地址
static struct mem_block *arena2block(struct arena *a, uint32_t idx) {
  return (struct mem_block *)((uint32_t)a + sizeof(struct arena) +
                              idx * a->desc->block_size);
}

// 返回内存块 b 所在的 arena 地址
static struct arena *block2arena(struct mem_block *b) {
  return (struct arena *)((uint32_t)b & 0xfffff000);
}

// 在堆中申请 size 字节内存
void *sys_malloc(uint32_t size) {
  PoolType PF;
  Pool *mem_pool;
  uint32_t pool_size;
  MemBlockDesc *descs;
  TaskStruct *cur_thread = running_thread();

  // 判断用哪个内存池
  if (cur_thread->pgdir == NULL) { // 若为内核线程
    PF = PF_KERNEL;
    pool_size = kernel_pool.size;
    mem_pool = &kernel_pool;
    descs = k_block_descs;
  } else { // 用户进程 pcb 中的 pgdir 会在为其分配页表时创建
    PF = PF_USER;
    pool_size = user_pool.size;
    mem_pool = &user_pool;
    descs = cur_thread->u_block_desc;
  }

  // 若申请的内存不在内存池容量范围内则直接返回 NULL
  if (!(size > 0 && size < pool_size)) {
    return NULL;
  }

  Arena *a;
  MemBlock *b;
  lock_acquire(&mem_pool->lock);

  // 超过最大内存块 1024, 就分配页框
  if (size > 1024) {
    uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PG_SIZE);

    a = malloc_page(PF, page_cnt);

    if (a != NULL) {
      memset(a, 0, page_cnt * PG_SIZE); // 将分配的内存清零
      // 对于分配的大块页框, 将 desc 置为 NULL, cnt 置为页框数, large 置为 true
      a->desc = NULL;
      a->cnt = page_cnt;
      a->large = true;
      lock_release(&mem_pool->lock);
      // 跨过 1个 Arena 结构体, 即返回跳过头部的剩余内存起始地址
      return (void *)(a + 1);
    } else {
      lock_release(&mem_pool->lock);
      return NULL;
    }
  } else { // 若申请的内存小于等于 1024, 可在各种规格的 mem_block_desc 中去适配
    uint8_t desc_idx;
    // 从内存块描述符中匹配合适的内存块规格
    for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
      if (size <= descs[desc_idx].block_size) { // 从小往大, 找到后退出
        break;
      }
    }

    // 若 mem_block_desc 的 free_list 中已经没有可用的 mem_block
    // 就创建新的 arena 提供 mem_block
    if (list_empty(&descs[desc_idx].free_list)) {
      a = malloc_page(PF, 1);
      if (a == NULL) {
        lock_release(&mem_pool->lock);
        return NULL;
      }
      memset(a, 0, PG_SIZE);

      // 对于分配的小块内存, 将 desc 置为相应内存块描述符
      // cnt 置为 arena 可用的内存块数, large 置为 false
      a->desc = &descs[desc_idx];
      a->large = false;
      a->cnt = descs[desc_idx].blocks_per_arena;
      uint32_t block_idx;

      IntrStatus old_status = intr_disable();

      // 开始将 arena 拆分成内存块, 并添加到内存块描述符的 free_list 中
      for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena;
           block_idx++) {
        b = arena2block(a, block_idx);
        ASSERT(!list_find(&a->desc->free_list, &b->free_elem));
        list_append(
            &a->desc->free_list,
            &b->free_elem); // 将当前新创建的块链接到全局的描述符的空闲链表之中
      }
      intr_set_status(old_status);
    }

    // 开始分配内存块
    b = elem2entry(MemBlock, free_elem, list_pop(&(descs[desc_idx].free_list)));
    memset(b, 0, descs[desc_idx].block_size);

    a = block2arena(b); // 获取内存块 b 所在的 arena
    a->cnt--;           // 将此 arena 中的空闲块数减 1
    lock_release(&mem_pool->lock);
    return (void *)b;
  }
}

// 回收内存 ptr
void sys_free(void *ptr) {
  ASSERT(ptr != NULL);
  if (ptr != NULL) {
    PoolType PF;
    Pool *mem_pool;

    // 判断是线程, 还是进程
    if (running_thread()->pgdir == NULL) {
      ASSERT((uint32_t)ptr >= K_HEAP_START);
      PF = PF_KERNEL;
      mem_pool = &kernel_pool;
    } else {
      PF = PF_USER;
      mem_pool = &user_pool;
    }

    lock_acquire(&mem_pool->lock);
    MemBlock *b = ptr;
    Arena *a = block2arena(b); // 转换成 arena, 获取元信息
    ASSERT(a->large == 0 || a->large == 1);
    if (a->desc == NULL && a->large == true) { // 大于 1024 的内存
      mfree_page(PF, a, a->cnt);
    } else { // 小于等于 1024 的内存块
      // 先将内存块回收到 free_list
      list_append(&a->desc->free_list, &b->free_elem);
      // 再判断此 arena 中的内存块是否都是空闲, 如果是就释放 arena
      if (++a->cnt == a->desc->blocks_per_arena) {
        uint32_t block_idx;
        for (block_idx = 0; block_idx < a->desc->blocks_per_arena;
             block_idx++) {
          MemBlock *b = arena2block(a, block_idx);
          ASSERT(list_find(&a->desc->free_list, &b->free_elem));
          list_remove(&b->free_elem);
        }
        mfree_page(PF, a, 1);
      }
    }
    lock_release(&mem_pool->lock);
  }
}

// 安装1页大小的vaddr,专门针对fork时虚拟地址位图无须操作的情况
void *get_a_page_without_opvaddrbitmap(PoolType pf, uint32_t vaddr) {
  Pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
  lock_acquire(&mem_pool->lock);
  void *page_phyaddr = palloc(mem_pool);
  if (page_phyaddr == NULL) {
    lock_release(&mem_pool->lock);
    return NULL;
  }
  page_table_add((void *)vaddr, page_phyaddr);
  lock_release(&mem_pool->lock);
  return (void *)vaddr;
}

// 根据物理页框地址 pg_phy_addr 在相应的内存池的位图清 0, 不改动页表
void free_a_phy_page(uint32_t pg_phy_addr) {
  Pool *mem_pool;
  uint32_t bit_idx = 0;
  if (pg_phy_addr >= user_pool.phy_addr_start) {
    mem_pool = &user_pool;
    bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PG_SIZE;
  } else {
    mem_pool = &kernel_pool;
    bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PG_SIZE;
  }
  bitmap_set(&mem_pool->pool_map, bit_idx, 0);
}

void mem_init() {
  // 在loader.S中使用BIOS方法获取了可用内存大小, 并写入total_men_bytes位置
  // 该位置为 LOADER_BASE_ADDR + 0x200 = 0x900 + 0x200 = 0xb00
  // 直接读取该位置数据, 即可获得可用内存大小
  uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
  mem_pool_init(mem_bytes_total);
  lock_init(&kernel_pool.lock);
  lock_init(&user_pool.lock);
  block_desc_init(k_block_descs);
}