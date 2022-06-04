#include "thread.h"
#include "memory.h"
#include "string.h"
#include <stdint.h>

#define PG_SIZE 4096

static void kernel_thread(thread_func *func, void *args) { func(args); }

void thread_create(TaskStruct *pthread, thread_func func, void *args) {

  pthread->self_kstack -= sizeof(IntrStack);

  pthread->self_kstack -= sizeof(ThreadStack);

  ThreadStack *ts = (ThreadStack *)pthread->self_kstack;
  ts->eip = kernel_thread;
  ts->function = func;
  ts->func_arg = args;
  ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
}

void init_thread(TaskStruct *pthread, char *name, int prio) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->status = TASK_RUNNING;
  pthread->priority = prio;
  pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->stack_magic = STACK_MAGIC;
}

TaskStruct *thread_start(char *name, int prio, thread_func func, void *args) {
  TaskStruct *thread = get_kernel_pages(1);
  init_thread(thread, name, prio);
  thread_create(thread, func, args);

  // 经过create函数后, thread->self_kstack指向ThreadStack的起始位置
  // 1. 将栈指针指向ThreadStack的起始位置
  // 2. 按照ThreadStack设置的值将寄存器的值弹出到指定的寄存器
  // 3. 将eip指向的地址作为返回地址, 执行返回操作
  // 上述过程相当于kernel_thread执行一个函数调用后返回到自己的环境之中, 从而实现了进入kernel_thread的效果
  __asm__ __volatile__ ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret"
          :
          : "g"(thread->self_kstack)
          : "memory");
    
    return thread;
}