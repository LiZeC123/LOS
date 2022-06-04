#include "time.h"
#include "debug.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "thread.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl,
                          uint8_t counter_mode, uint16_t counter_value) {
  outb(PIT_CONTROL_PORT,
       (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
  outb(counter_port, (uint8_t)counter_value);
  outb(counter_port, (uint8_t)counter_value >> 8);
}

uint32_t ticks; // 内核开启中断以来, 经过的tick总数

static void intr_timer_handler() {
  TaskStruct *cur_thead = running_thread();

  ASSERT(cur_thead->stack_magic == STACK_MAGIC);

  cur_thead->elapsed_ticks++;
  ticks++;

  if (cur_thead->ticks == 0) {
    schedule();
  } else {
    cur_thead->ticks--;
  }
}

void time_init() {
  put_str("timer_init start\n");
  frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
                COUNTER0_VALUE);
  register_handler(0x20, intr_timer_handler);
  put_str("timer_init done\n");
}
