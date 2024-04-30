#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"

#define IDT_DESC_CNT 0x81

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

char *intr_name[IDT_DESC_CNT]; // 保存异常的名字

extern intr_handler
    intr_entry_table[IDT_DESC_CNT]; // 引用kernel.S中定义的中断向量入口数组

intr_handler idt_table[IDT_DESC_CNT]; // C语言实现的中断处理函数的列表,
// 发生中断时进入intr_entry_table,
// 通过汇编代码保存环境后根据中断号计算idt_table的偏移量并跳转执行
// 因此在idt_table需要填入合适的函数地址

static void make_idt_desc(GateDesc *p_gdesc, uint8_t attr,
                          intr_handler function) {
  p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
  p_gdesc->selector = SELECTOR_K_CODE;
  p_gdesc->dcount = 0;
  p_gdesc->attribute = attr;
  p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;
}

extern uint32_t syscall_handler(void);

// 初始化中断向量表
static void idt_desc_init() {
  for (int i = 0; i < IDT_DESC_CNT; i++) {
    make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
  }

  // 单独注册 系统调用的中断
  make_idt_desc(&idt[0x80], IDT_DESC_ATTR_DPL3, syscall_handler);

  put_str("  idt_desc_init done\n");
}

static void general_intr_handler(uint8_t vec_nr) {
  if (vec_nr == 0x27 || vec_nr == 0x2f) {
    return;
  }
  put_str("int vector: ");
  print_number(vec_nr);
  put_str(" ");
  put_str(intr_name[vec_nr]);
  next_line();

  if (vec_nr == 14) {
    int page_fault_vaddr = 0;
    __asm__ __volatile__("movl %%cr2, %0" : "=r"(page_fault_vaddr));
    PRINTLINE("Page Fault Addr is ", page_fault_vaddr);
  }

  while (1) {
    // 需要处理的中断都会单独处理, 如果出现其他情况, 则停住以便于调试
  }
}

static void exception_init(void) {
  for (int i = 0; i < IDT_DESC_CNT; i++) {
    // 先将所有的中断统一处理
    idt_table[i] = general_intr_handler;
    intr_name[i] = "unknown";
  }

  // 然后根据规则赋予实际的值
  intr_name[0] = "#DE Divide Error";
  intr_name[1] = "#DB Debug Exception";
  intr_name[2] = "NMI Interrupt";
  intr_name[3] = "#BP Breakpoint Exception";
  intr_name[4] = "#OF Overflow Exception";
  intr_name[5] = "#BR BOUND Range Exceeded Exception";
  intr_name[6] = "#UD Invalid Opcode Exception";
  intr_name[7] = "#NM Device Not Available Exception";
  intr_name[8] = "#DF Double Fault Exception";
  intr_name[9] = "Coprocessor Segment Overrun";
  intr_name[10] = "#TS Invalid TSS Exception";
  intr_name[11] = "#NP Segment Not Present";
  intr_name[12] = "#SS Stack Fault Exception";
  intr_name[13] = "#GP General Protection Exception";
  intr_name[14] = "#PF Page-Fault Exception";
  // intr_name[l5]第15项是intel保留项，未使用
  intr_name[16] = "#MF x87 FPU F'loating-Point Error";
  intr_name[17] = "#AC Alignment Check Exception";
  intr_name[18] = "#MC Machine-Check Exception";
  intr_name[19] = "#XF SIMD Floating-Point Exception";
}

void register_handler(uint8_t vector_no, intr_handler function) {
  idt_table[vector_no] = function;
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

  outb(PIC_M_DATA, 0xf8);
  outb(PIC_S_DATA, 0xbf);

  // // 打开键盘中断
  // outb(PIC_M_DATA, 0xfd);
  // outb(PIC_S_DATA, 0xff);

  put_str("  pic_init done\n");
}

#define EFLAGS_IF 0x00000200
#define GET_EFLAGS(EFLAG_VAR)                                                  \
  __asm__ __volatile__("pushfl; popl %0" : "=g"(EFLAG_VAR))

IntrStatus intr_get_status() {
  uint32_t eflags = 0;
  GET_EFLAGS(eflags);
  return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

IntrStatus intr_enable() {
  if (INTR_ON == intr_get_status()) {
    return INTR_ON;
  } else {
    __asm__ __volatile__("sti");
    return INTR_OFF;
  }
}

IntrStatus intr_disable() {
  if (INTR_ON == intr_get_status()) {
    __asm__ __volatile__("cli" : : : "memory");
    return INTR_ON;
  } else {
    return INTR_OFF;
  }
}

IntrStatus intr_set_status(IntrStatus status) {
  return status & INTR_ON ? intr_enable() : intr_disable();
}

void idt_init() {
  put_str("idt_init start\n");
  idt_desc_init();
  exception_init();
  pic_init();

  // idt数组变量实际上就是一个地址值
  // 将地址先转化为32位数字, 再转化为64位数字, 避免直接转换产生的warning
  uint64_t idt_operand = (sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt) << 16;
  __asm__ __volatile__("lidt %0" : : "m"(idt_operand));

  put_str("idt_init done\n");
}