#include "time.h"
#include "debug.h"
#include "func.h"
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

#define IRQ0_FREQUENCY 100
#define MS_PER_INTR (1000 / IRQ0_FREQUENCY)

// 以 tick 为单位的 sleep, 任何时间形式的 sleep 会转换此 ticks 形式
static void ticks_to_sleep(uint32_t sleep_ticks) {
  uint32_t start_tick = ticks;
  // 若间隔的 ticks 数不够便让出 cpu
  while (ticks - start_tick < sleep_ticks) {
    thread_yield();
  }
}

// 以毫秒为单位的 sleep
void mtime_sleep(uint32_t m_seconds) {
  uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds, MS_PER_INTR);
  ASSERT(sleep_ticks > 0);
  ticks_to_sleep(sleep_ticks);
}

void time_init() {
  put_str("timer_init ... ");
  frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
                COUNTER0_VALUE);
  register_handler(0x20, intr_timer_handler);
  put_str("done\n");
}
