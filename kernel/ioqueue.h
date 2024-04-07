#pragma once

#include "sync.h"
#include "thread.h"
#include <stdint.h>

#define BUF_SIZE 64

// 环形阻塞队列
typedef struct ioqueue {
  Lock lock;
  TaskStruct *producer;
  TaskStruct *consumer;
  char buf[BUF_SIZE];

  int32_t head; // 注意: 每次在head指向的位置写入数据
  int32_t tail;
} IoQueue;

void ioqueue_init(IoQueue *ioq);
bool ioq_full(IoQueue *ioq);
bool ioq_empty(IoQueue *ioq);
char ioq_getchar(IoQueue *ioq);
void ioq_putchar(IoQueue *ioq, char byte);
