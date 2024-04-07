#include "ioqueue.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"

void ioqueue_init(IoQueue *ioq) {
  lock_init(&ioq->lock);
  ioq->producer = ioq->consumer = NULL;
  ioq->head = ioq->tail = 0;
}

static int32_t next_pos(int32_t pos) { return (pos + 1) % BUF_SIZE; }

// 判断队列是否已满
bool ioq_full(IoQueue *ioq) {
  ASSERT(intr_get_status() == INTR_OFF);
  return next_pos(ioq->head) == ioq->tail;
}

// 判断队列是否已空
bool ioq_empty(IoQueue *ioq) {
  ASSERT(intr_get_status() == INTR_OFF);
  return ioq->head == ioq->tail;
}

// 使当前生产者或消费者在此缓冲区上等待
static void ioq_wait(TaskStruct **waiter) {
  ASSERT(*waiter == NULL && waiter != NULL);
  *waiter = running_thread();
  thread_block(TASK_BLOCKED);
}

// 唤醒 waiter
static void wakeup(TaskStruct **waiter) {
  ASSERT(*waiter != NULL);
  thread_unblock(*waiter);
  *waiter = NULL;
}

// 消费者从 ioq 队列中获取一个字符
char ioq_getchar(IoQueue *ioq) {
  ASSERT(intr_get_status() == INTR_OFF);

  while (ioq_empty(ioq)) {
    lock_acquire(&ioq->lock);
    ioq_wait(&ioq->consumer);
    lock_release(&ioq->lock);
  }

  char byte = ioq->buf[ioq->tail];
  ioq->tail = next_pos(ioq->tail);

  if (ioq->producer != NULL) {
    wakeup(&ioq->producer);
  }

  return byte;
}

// 生产者往 ioq 队列中写入一个字符 byte
void ioq_putchar(IoQueue *ioq, char byte) {
  ASSERT(intr_get_status() == INTR_OFF);

  while (ioq_full(ioq)) {
    lock_acquire(&ioq->lock);
    ioq_wait(&ioq->producer);
    lock_release(&ioq->lock);
  }

  ioq->buf[ioq->head] = byte;
  ioq->head = next_pos(ioq->head);

  if (ioq->consumer != NULL) {
    wakeup(&ioq->consumer);
  }
}