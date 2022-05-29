#include <stdint.h>

#include "global.h"
#include "io.h"
#include "print.h"

#define IDT_DESC_CNT 0x21

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

typedef struct {
  uint16_t func_offset_low_word;
  uint16_t selector;
  uint8_t dcount;

  uint8_t attribute;
  uint16_t func_offset_high_word;
} GateDesc;

// 中断描述符表
static GateDesc idt[IDT_DESC_CNT];

typedef void* intr_handler;

extern intr_handler
    intr_entry_table[IDT_DESC_CNT];  // 引用kernel.S中定义的中断向量入口数组

static void make_idt_desc(GateDesc* p_gdesc, uint8_t attr, intr_handler function) {

  p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
  p_gdesc->selector = SELECTOR_K_CODE;
  p_gdesc->dcount = 0;
  p_gdesc->attribute = attr;
  p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;
}

// 初始化中断向量表
static void idt_desc_init() {
  for (int i = 0; i < IDT_DESC_CNT; i++) {
    make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
  }
  put_str("  idt_desc_init done\n");
}

// 初始化8259A
static void pic_init() {
  outb(PIC_M_CTRL, 0x11);
  outb(PIC_M_DATA, 0x20);
  outb(PIC_M_DATA, 0x04);
  outb(PIC_M_DATA, 0x01);

  outb(PIC_S_CTRL, 0x11);
  outb(PIC_S_DATA, 0x28);
  outb(PIC_S_DATA, 0x02);
  outb(PIC_S_DATA, 0x01);

  outb(PIC_M_DATA, 0xfe);
  outb(PIC_S_DATA, 0xff);

  put_str("  pic_init done\n");
}

void idt_init() {
  put_str("idt_init start\n");
  idt_desc_init();
  pic_init();

  // idt数组变量实际上就是一个地址值
  // 将地址先转化为32位数字, 再转化为64位数字, 避免直接转换产生的warning
  uint64_t idt_operand = (sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt) << 16;
  __asm__ __volatile__("lidt %0" : : "m"(idt_operand));

  put_str("idt_init done\n");
}