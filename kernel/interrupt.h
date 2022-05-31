#pragma once




void idt_init();



// 中断状态相关函数
typedef enum { INTR_OFF, INTR_ON } IntrStatus;

IntrStatus intr_set_status(IntrStatus status);
IntrStatus intr_get_status();
IntrStatus intr_enable();
IntrStatus intr_disable();
