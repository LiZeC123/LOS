#include "process.h"
#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "memory.h"
#include "string.h"
#include "tss.h"

#define PG_SIZE 4096

extern void intr_exit(void);

void start_process(void *filename_) {
  void *function = filename_;
  TaskStruct *cur = running_thread();
  cur->self_kstack += sizeof(ThreadStack);
  IntrStack *is = (IntrStack *)cur->self_kstack;
  is->edi = is->esi = is->ebp = is->esp_dummy = 0;

  is->ebx = is->edx = is->ecx = is->eax = 0;

  is->gs = 0;
  is->ds = is->es = is->fs = SELECTOR_U_DATA;
  is->eip = function;
  is->cs = SELECTOR_U_CODE;
  is->eflags = (EFLAG_IOPL_0 | EFLAG_MBS | EFLAGS_IF_1);
  is->esp =
      (void *)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
  is->ss = SELECTOR_U_DATA;
  __asm__ __volatile__("movl %0, %%esp; jmp intr_exit" ::"g"(is) : "memory");
}

// 切换页目录表
// 不同进程之间使用不同的页目录表, 因此需要切换CR3寄存器的值
void page_dir_activate(TaskStruct *p_thread) {
  uint32_t pagedir_phy_addr = 0x100000; // 内核线程的默认页目录表地址
  if (p_thread->pgdir != NULL) {
    pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);
  }

  __asm__ __volatile__("movl %0, %%cr3" ::"r"(pagedir_phy_addr) : "memory");
}

void process_active(TaskStruct *p_thread) {
  ASSERT(p_thread != NULL);
  page_dir_activate(p_thread);

  if (p_thread->pgdir) {
    update_tss_esp(p_thread);
  }
}

uint32_t *create_page_dir() {
  // 用户进程的页表不应该被用户进程访问到, 因此在内核空间中申请
  // 首先申请页目录表的空间, 其大小刚好为1页
  uint32_t *page_dir_vaddr = get_kernel_pages(1);
  if (page_dir_vaddr == NULL) {
    console_put_str("create_page_dir: get_kernel_gate failed!");
    return NULL;
  }

  // 把页目录表中表示内核1GB的页目录项复制到用户的页目录表之中
  // 即第 0x300 ~ 0x399 项
  memcpy((uint32_t *)((uint32_t)page_dir_vaddr + 0x300 * 4),
         (uint32_t *)(0xfffff000 + 0x300 * 4), 1024);

  // 页目录表最后一项指向自己
  uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
  page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;

  return page_dir_vaddr;
}

void create_user_vaddr_bitmap(TaskStruct *user_prog) {
  user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;
  uint32_t user_map_len = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;

  uint32_t bitmap_pg_cnt = DIV_ROUND_UP(user_map_len, PG_SIZE);
  user_prog->userprog_vaddr.vaddr_map.bits = get_kernel_pages(bitmap_pg_cnt);

  user_prog->userprog_vaddr.vaddr_map.btmp_bytes_len = user_map_len;

  bitmap_init(&user_prog->userprog_vaddr.vaddr_map);
}

extern List thread_ready_list;
extern List thread_all_list;

void process_execute(void *filename, char *name) {
  TaskStruct *thread = get_kernel_pages(1);
  init_thread(thread, name, DEFAULT_PRIO);
  create_user_vaddr_bitmap(thread);
  thread_create(thread, start_process, filename);
  thread->pgdir = create_page_dir();
  block_desc_init(thread->u_block_desc);

  IntrStatus old_status = intr_disable();
  ASSERT(!list_find(&thread_ready_list, &thread->general_tag));
  list_append(&thread_ready_list, &thread->general_tag);

  ASSERT(!list_find(&thread_all_list, &thread->all_list_tag));
  list_append(&thread_all_list, &thread->all_list_tag);
  intr_set_status(old_status);
}