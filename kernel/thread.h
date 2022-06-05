#pragma once

#include <stdint.h>
#include "list.h"

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

typedef struct {
  uint32_t *self_kstack;
  TaskStatus status;
  uint8_t priority;
  char name[16];

  uint8_t ticks;
  uint32_t elapsed_ticks;

  ListElem general_tag;   
  ListElem all_list_tag;
  uint32_t* pgdir;    // 进程自己的页表的虚拟地址

  uint32_t stack_magic;
} TaskStruct;

#define STACK_MAGIC 0xcafebabe


TaskStruct *thread_start(char *name, int prio, thread_func func, void *args);

TaskStruct *running_thread();

void schedule();

void thread_init();

void thread_block(TaskStatus stat);

void thread_unblock(TaskStruct *pthread);