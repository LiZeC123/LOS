#pragma once

#include "list.h"
#include <stdint.h>
#include "losmemory.h"

typedef void thread_func(void *);

typedef enum {
  TASK_RUNNING,
  TASK_READY,
  TASK_BLOCKED,
  TASK_WAITING,
  TASK_HANGING,
  TASK_DIED
} TaskStatus;

typedef struct {
  uint32_t vec_no;
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp_dummy;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;

  // cpu从低特权级进入高特权级时压入
  uint32_t err_code;
  void (*eip)(void);
  uint32_t cs;
  uint32_t eflags;
  void *esp;
  uint32_t ss;

} IntrStack;

typedef struct {
  uint32_t ebp;
  uint32_t ebx;
  uint32_t edi;
  uint32_t esi;

  // 以下部分用于在内存中构建一个函数调用的状态
  void (*eip)(thread_func *func, void *func_arg);
  void(*unused_retaddr); // 函数返回地址
  thread_func *function; // ebp+4 第一个参数
  void *func_arg;        // ebp+8 第二个参数

} ThreadStack;

typedef int16_t pid_t;

#define MAX_FILES_OPEN_PER_PROC 8

typedef struct {
  uint32_t *self_kstack;  // 线程的内核栈
  pid_t pid;
  TaskStatus status;
  uint8_t priority;
  char name[16];

  uint8_t ticks;
  uint32_t elapsed_ticks;

  int32_t fd_table[MAX_FILES_OPEN_PER_PROC]; // 文件描述符数组

  ListElem general_tag;
  ListElem all_list_tag;

  uint32_t *pgdir;            // 进程自己的页表的虚拟地址
  VirtualAddr userprog_vaddr; // 用户进程的虚拟地址
  MemBlockDesc u_block_desc[DESC_CNT];

  uint32_t stack_magic; // 记录PCB边界信息, 从而可检查是否被破坏
} TaskStruct;

#define STACK_MAGIC 0xcafebabe

TaskStruct *thread_start(char *name, int prio, thread_func func, void *args);

TaskStruct *running_thread();

void schedule();

void thread_init();

void thread_block(TaskStatus stat);

void thread_unblock(TaskStruct *pthread);

void thread_yield();

void thread_create(TaskStruct *pthread, thread_func func, void *args);
void init_thread(TaskStruct *pthread, char *name, int prio);