#include "sync.h"
#include "debug.h"
#include "interrupt.h"

void sema_init(Semaphore *sm, uint8_t value) {
  sm->value = value;
  list_init(&sm->waiters);
}

void lock_init(Lock *lock) {
  lock->holder = NULL;
  lock->holder_repeat_nr = 0;
  sema_init(&lock->semaphore, 1);
}

void sema_down(Semaphore *sm) {
  IntrStatus old_status = intr_disable();
  // 如果无法获得信号量, 则挂起当前线程
  while (sm->value == 0) {
    // 当前线程不应该已经在等待队列中
    ASSERT(!list_find(&sm->waiters, &running_thread()->general_tag));

    list_append(&sm->waiters, &running_thread()->general_tag);
    thread_block(TASK_BLOCKED);
  }

  // 否则说明可以申请信号量
  sm->value--;
  ASSERT(sm->value == 0);

  intr_set_status(old_status);
}

void sema_up(Semaphore *sm) {
  IntrStatus old_status = intr_disable();
  ASSERT(sm->value == 0);

  if (!list_empty(&sm->waiters)) {
    TaskStruct *thread_blocked =
        elem2entry(TaskStruct, general_tag, list_pop(&sm->waiters));

    thread_unblock(thread_blocked);
  }

  sm->value++;
  ASSERT(sm->value == 1);

  intr_set_status(old_status);
}

void lock_acquire(Lock *lock) {
  if (lock->holder != running_thread()) {
    // 如果当前线程并没有获得这个锁, 则尝试获取这个锁
    sema_down(&lock->semaphore);
    lock->holder = running_thread();
    ASSERT(lock->holder_repeat_nr == 0);
    lock->holder_repeat_nr = 1;
  } else {
    // 否则可以直接重入
    lock->holder_repeat_nr++;
  }
}

void lock_release(Lock *lock) {
  ASSERT(lock->holder == running_thread());
  if (lock->holder_repeat_nr > 1) {
    lock->holder_repeat_nr--;
    return;
  }

  ASSERT(lock->holder_repeat_nr == 1);

  lock->holder = NULL;
  lock->holder_repeat_nr = 0;
  sema_up(&lock->semaphore);
}