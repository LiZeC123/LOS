#pragma once

#include "list.h"
#include "thread.h"
#include <stdint.h>

typedef struct {
  uint8_t value;
  List waiters;
} Semaphore;

typedef struct {
  TaskStruct *holder;
  Semaphore semaphore;
  uint32_t holder_repeat_nr; // 持有者重复申请的次数
} Lock;

void sema_init(Semaphore *sm, uint8_t value);

void sema_down(Semaphore *sm);

void sema_up(Semaphore *sm);

void lock_init(Lock *lock);

void lock_acquire(Lock *lock);

void lock_release(Lock *lock);