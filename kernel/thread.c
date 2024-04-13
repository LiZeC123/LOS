#include "thread.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "string.h"
#include "sync.h"

#define PG_SIZE 4096

TaskStruct *main_thread;
List thread_ready_list;
List thread_all_list;

static ListElem *thread_tag; //

extern void switch_to(TaskStruct *cur, TaskStruct *next);

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
static pid_t allocate_pid(void) {
  static pid_t next_pid = 0;
  lock_acquire(&pid_lock);
  next_pid++;
  lock_release(&pid_lock);
  return next_pid;
}

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

  pthread->stack_magic = STACK_MAGIC;
}

TaskStruct *thread_start(char *name, int prio, thread_func func, void *args) {
  TaskStruct *thread = get_kernel_pages(1);
  init_thread(thread, name, prio);
  thread_create(thread, func, args);

  ASSERT(!list_find(&thread_ready_list, &thread->general_tag));
  list_append(&thread_ready_list, &thread->general_tag);

  ASSERT(!list_find(&thread_all_list, &thread->all_list_tag))
  list_append(&thread_all_list, &thread->all_list_tag);

  return thread;
}

static void make_main_thread() {
  main_thread = running_thread();
  init_thread(main_thread, "main", 31);

  // 主线程已经运行, 因此仅进入all队列
  ASSERT(!list_find(&thread_all_list, &main_thread->all_list_tag))
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

  ASSERT(!list_empty(&thread_ready_list));

  // 从就绪队列中调度下一个线程
  thread_tag = list_pop(&thread_ready_list);
  TaskStruct *next = elem2entry(TaskStruct, general_tag, thread_tag);
  next->status = TASK_RUNNING;

  process_active(next); // 激活下一个任务的页表
  switch_to(cur, next);
}

void thread_init() {
  put_str("thread_init start\n");
  list_init(&thread_all_list);
  list_init(&thread_ready_list);
  lock_init(&pid_lock);
  make_main_thread();
  put_str("thread_init done\n");
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
