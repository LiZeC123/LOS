#pragma once

#include <stdint.h>

void idt_init();

// 中断状态相关函数
typedef enum { INTR_OFF, INTR_ON } IntrStatus;

typedef void *intr_handler;

IntrStatus intr_set_status(IntrStatus status);
IntrStatus intr_get_status();
IntrStatus intr_enable();
IntrStatus intr_disable();

void register_handler(uint8_t vector_no, intr_handler function);

// 中断返回处理函数, 使用汇编实现, 此处仅声明
void intr_exit(void);