#include "thread.h"
#include "debug.h"
#include "file.h"
#include "func.h"
#include "interrupt.h"
#include "list.h"
#include "print.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "sync.h"

#define PG_SIZE 4096

TaskStruct *main_thread, *idle_thread;
List thread_ready_list;
List thread_all_list;

static ListElem *thread_tag; //

// 线程切换函数, 在汇编中实现
void switch_to(TaskStruct *cur, TaskStruct *next);

static void kernel_thread(thread_func *func, void *args) {
  // 时钟中断进行调度后, 会自动关闭中断
  // 因此在执行线程的函数之前, 需要重新开启中断, 使该线程可以被中断
  intr_enable();
  func(args);
}

void thread_create(TaskStruct *pthread, thread_func func, void *args) {

  pthread->self_kstack -= sizeof(IntrStack);

  pthread->self_kstack -= sizeof(ThreadStack);

  ThreadStack *ts = (ThreadStack *)pthread->self_kstack;
  ts->eip = kernel_thread;
  ts->function = func;
  ts->func_arg = args;
  ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
}

// 分配 pid
Lock pid_lock;
static pid_t allocate_pid() {
  static pid_t next_pid = 0;
  lock_acquire(&pid_lock);
  next_pid++;
  lock_release(&pid_lock);
  return next_pid;
}

pid_t fork_pid() { return allocate_pid(); }

// 初始化线程指针（PCB），线程名称，线程优先级
void init_thread(TaskStruct *pthread, char *name, int prio) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->pid = allocate_pid();

  if (pthread == main_thread) {
    pthread->status = TASK_RUNNING;
  } else {
    pthread->status = TASK_READY;
  }

  pthread->priority = prio;
  pthread->ticks = prio;
  pthread->elapsed_ticks = 0;
  pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->pgdir = NULL;

  // 预留标准输入输出
  pthread->fd_table[0] = 0;
  pthread->fd_table[1] = 1;
  pthread->fd_table[2] = 2;
  // 其余的全置为 -1
  uint8_t fd_idx = 3;
  while (fd_idx < MAX_FILES_OPEN_PER_PROC) {
    pthread->fd_table[fd_idx] = -1;
    fd_idx++;
  }

  pthread->cwd_inode_nr = 0;
  pthread->parent_pid = -1; // -1表示无父进程
  pthread->stack_magic = STACK_MAGIC;
}

TaskStruct *thread_start(char *name, int prio, thread_func func, void *args) {
  TaskStruct *thread = get_kernel_pages(1);
  init_thread(thread, name, prio);
  thread_create(thread, func, args);

  ASSERT(!list_find(&thread_ready_list, &thread->general_tag));
  list_append(&thread_ready_list, &thread->general_tag);

  ASSERT(!list_find(&thread_all_list, &thread->all_list_tag));
  list_append(&thread_all_list, &thread->all_list_tag);

  return thread;
}

static void make_main_thread() {
  main_thread = running_thread();
  init_thread(main_thread, "main", 31);

  // 主线程已经运行, 因此仅进入all队列
  ASSERT(!list_find(&thread_all_list, &main_thread->all_list_tag));
  list_append(&thread_all_list, &main_thread->all_list_tag);
}

// 获得当前线程的PCB指针
TaskStruct *running_thread() {
  uint32_t esp;
  __asm__ __volatile__("mov %%esp, %0" : "=g"(esp));
  // 由于每个PCB都是当前内存页的起始位置, 因此直接对地址取整数即可
  return (TaskStruct *)(esp & 0xfffff000);
}

void schedule() {
  ASSERT(intr_get_status() == INTR_OFF);

  TaskStruct *cur = running_thread();
  if (cur->status == TASK_RUNNING) {
    // 如果是时间片耗尽, 则将当前线程重新放入就绪队列
    ASSERT(!list_find(&thread_ready_list, &cur->general_tag));
    list_append(&thread_ready_list, &cur->general_tag);

    cur->ticks = cur->priority;
    cur->status = TASK_READY;
  } else {
    // 否则其他情况, 该线程无法就绪, 留在all队列即可
  }

  if (list_empty(&thread_ready_list)) {
    // 如果就绪队列中没有可运行的任务, 就唤醒 idle
    thread_unblock(idle_thread);
  }

  // 从就绪队列中调度下一个线程
  thread_tag = list_pop(&thread_ready_list);
  TaskStruct *next = elem2entry(TaskStruct, general_tag, thread_tag);
  next->status = TASK_RUNNING;

  process_active(next); // 激活下一个任务的页表
  switch_to(cur, next);
}

void thread_block(TaskStatus stat) {
  ASSERT(stat == TASK_BLOCKED || stat == TASK_WAITING || stat == TASK_HANGING);

  IntrStatus old_status = intr_disable();
  TaskStruct *cur_thread = running_thread();
  cur_thread->status = stat;

  schedule(); // 执行调度, 从而将当前线程换下处理器

  // 从阻塞解除后才能执行后续的代码
  intr_set_status(old_status);
}

void thread_unblock(TaskStruct *pthread) {
  IntrStatus old_status = intr_disable();

  TaskStatus stat = pthread->status;
  ASSERT(stat == TASK_BLOCKED || stat == TASK_WAITING || stat == TASK_HANGING);

  if (stat != TASK_READY) {
    ASSERT(!list_find(&thread_ready_list, &pthread->general_tag));
    list_push(&thread_ready_list, &pthread->general_tag);
    pthread->status = TASK_READY;
  }

  intr_set_status(old_status);
}

void thread_yield() {
  TaskStruct *cur = running_thread();
  IntrStatus oldStatus = intr_disable();
  ASSERT(!list_find(&thread_ready_list, &cur->general_tag));
  list_append(&thread_ready_list, &cur->general_tag);
  cur->status = TASK_READY;
  schedule();
  intr_set_status(oldStatus);
}

static void idle(void *arg) {
  UNUSED(arg);
  while (1) {
    thread_block(TASK_BLOCKED);
    // 执行 hlt 时必须要保证目前处在开中断的情况下
    __asm__ __volatile__("sti; hlt" : : : "memory");
  }
}

/* 以填充空格的方式输出buf */
static void pad_print(char *buf, int32_t buf_len, void *ptr, char format) {
  memset(buf, 0, buf_len);
  uint8_t out_pad_0idx = 0;
  switch (format) {
  case 's':
    out_pad_0idx = sprintf(buf, "%s", ptr);
    break;
  case 'd':
    out_pad_0idx = sprintf(buf, "%d", *((int16_t *)ptr));
    break;
  case 'x':
    out_pad_0idx = sprintf(buf, "%x", *((uint32_t *)ptr));
  }
  while (out_pad_0idx < buf_len) { // 以空格填充
    buf[out_pad_0idx] = ' ';
    out_pad_0idx++;
  }
  sys_write(stdout_no, buf, buf_len - 1);
}

/* 用于在list_traversal函数中的回调函数,用于针对线程队列的处理 */
static bool elem2thread_info(ListElem *pelem, int arg) {
  UNUSED(arg);
  TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
  char out_pad[16] = {0};

  pad_print(out_pad, 16, &pthread->pid, 'd');

  if (pthread->parent_pid == -1) {
    pad_print(out_pad, 16, "NULL", 's');
  } else {
    pad_print(out_pad, 16, &pthread->parent_pid, 'd');
  }

  switch (pthread->status) {
  case 0:
    pad_print(out_pad, 16, "RUNNING", 's');
    break;
  case 1:
    pad_print(out_pad, 16, "READY", 's');
    break;
  case 2:
    pad_print(out_pad, 16, "BLOCKED", 's');
    break;
  case 3:
    pad_print(out_pad, 16, "WAITING", 's');
    break;
  case 4:
    pad_print(out_pad, 16, "HANGING", 's');
    break;
  case 5:
    pad_print(out_pad, 16, "DIED", 's');
  }
  pad_print(out_pad, 16, &pthread->elapsed_ticks, 'x');

  memset(out_pad, 0, 16);
  ASSERT(strlen(pthread->name) < 17);
  memcpy(out_pad, pthread->name, strlen(pthread->name));
  strcat(out_pad, "\n");
  sys_write(stdout_no, out_pad, strlen(out_pad));
  return false; // 此处返回false是为了迎合主调函数list_traversal,只有回调函数返回false时才会继续调用此函数
}

/* 打印任务列表 */
void sys_ps(void) {
  char *ps_title =
      "PID            PPID           STAT           TICKS          COMMAND\n";
  sys_write(stdout_no, ps_title, strlen(ps_title));
  list_traversal(&thread_all_list, elem2thread_info, 0);
}

// 回收 thread_over 的 pcb 和页表, 并将其从调度队列中去除
void thread_exit(TaskStruct *thread_over, bool need_schedule) {
  // 要保证 schedule 在关中断情况下调用
  intr_disable();
  thread_over->status = TASK_DIED;

  // 如果 thread_over 不是当前线程, 就有可能还在就绪队列中, 将其从中删除
  if (list_find(&thread_ready_list, &thread_over->general_tag)) {
    list_remove(&thread_over->general_tag);
  }
  if (thread_over->pgdir) { // 如果是进程, 回收进程的页表
    mfree_page(PF_KERNEL, thread_over->pgdir, 1);
  }

  // 从 all_thread_list 中去掉此任务
  list_remove(&thread_over->all_list_tag);

  // 回收 pcb 所在的页, 主线程的 pcb 不在堆中, 跨过
  if (thread_over != main_thread) {
    mfree_page(PF_KERNEL, thread_over, 1);
  }

  // // 归还 pid
  // release_pid(thread_over->pid);

  // 如果需要下一轮调度则主动调用 schedule
  if (need_schedule) {
    schedule();
    PANIC("thread_exit: should not be here\n");
  }
}

// 比对任务的 pid
static bool pid_check(ListElem *pelem, int32_t pid) {
  TaskStruct *pthread = elem2entry(TaskStruct, all_list_tag, pelem);
  return pthread->pid == pid;
}

// 根据 pid 找 pcb, 若找到则返回该 pcb, 否则返回 NULL
TaskStruct *pid2thread(int32_t pid) {
  ListElem *pelem = list_traversal(&thread_all_list, pid_check, pid);
  if (pelem == NULL) {
    return NULL;
  }
  TaskStruct *thread = elem2entry(TaskStruct, all_list_tag, pelem);
  return thread;
}

// 声明init函数, 对应的实现在main文件
void init();

void thread_init() {
  put_str("thread_init ... ");
  list_init(&thread_all_list);
  list_init(&thread_ready_list);
  lock_init(&pid_lock);
  process_execute(init, "init");
  make_main_thread();
  idle_thread = thread_start("idle", 10, idle, NULL);
  put_str("done\n");
}