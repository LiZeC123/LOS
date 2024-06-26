#include "bitmap.h"
#include "debug.h"
#include "string.h"

// 初始化位图
void bitmap_init(BitMap *m) {
  ASSERT(m != NULL);

  memset(m->bits, 0, m->btmp_bytes_len);
}

// 判断给定位置是否为1
bool bitmap_scan_test(BitMap *m, uint32_t idx) {
  ASSERT(m != NULL || m->btmp_bytes_len * 8 < idx);

  uint32_t byte_idx = idx / 8;
  uint32_t bit_idx = idx % 8;

  return m->bits[byte_idx] & (BITMAP_MASK << bit_idx);
}

// 向位图申请cnt个bit, 成功则返回起始下标, 失败则返回-1
int bitmap_scan(BitMap *m, uint32_t cnt) {
  ASSERT(m != NULL);

  uint32_t idx = 0;
  while (0xff == m->bits[idx] && (idx < m->btmp_bytes_len)) {
    idx++;
  }

  if (idx == m->btmp_bytes_len) {
    return -1;
  }

  int bit_idx = 0;
  while ((uint8_t)(BITMAP_MASK << bit_idx) & m->bits[idx]) {
    bit_idx++;
  }

  int start = idx * 8 + bit_idx;
  if (cnt == 1) {
    // 如果仅需要申请1页空间, 则可以直接返回
    return start;
  }

  // 否则需要判断从start开始的位置是否具有cnt个空闲空间
  uint32_t bit_left = m->btmp_bytes_len * 8 - start;
  uint32_t next_bit = start + 1;
  uint32_t count = 1;

  start = -1;
  while (bit_left-- > 0) {
    if (!bitmap_scan_test(m, next_bit)) {
      count++;
    } else {
      count = 0;
    }

    if (count == cnt) {
      start = next_bit - count + 1;
      break;
    }
    next_bit++;
  }
  return start;
}

// 设置位图给定位置的值
void bitmap_set(BitMap *m, uint32_t idx, int8_t value) {
  ASSERT(m != NULL || m->btmp_bytes_len * 8 < idx);
  ASSERT((value == 0) || (value == 1));

  uint32_t byte_idx = idx / 8;
  uint32_t bit_idx = idx % 8;

  if (value) {
    m->bits[byte_idx] |= (BITMAP_MASK << bit_idx);
  } else {
    m->bits[byte_idx] &= ~(BITMAP_MASK << bit_idx);
  }
}

#ifdef TEST
#include <stdio.h>

void bitmap_print(BitMap *m) {
  printf("BitMap@0x%x: [", m);
  for (unsigned int i = 0; i < m->btmp_bytes_len; i++) {
    printf("%x ", m->bits[i]);
  }
  printf("]\n");
}

#endif