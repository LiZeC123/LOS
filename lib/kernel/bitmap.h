#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BITMAP_MASK 1

typedef struct {
  uint32_t btmp_bytes_len;
  uint8_t *bits;
} BitMap;

// 初始化位图
void bitmap_init(BitMap *m);

// 判断给定位置是否为1
bool bitmap_scan_test(BitMap *m, uint32_t idx);

// 向位图申请cnt个bit, 成功则返回起始下标, 失败则返回-1
int bitmap_scan(BitMap *m, uint32_t cnt);

// 设置位图给定位置的值
void bitmap_set(BitMap *m, uint32_t idx, int8_t value);

#ifdef TEST
// 打印内部数组
void bitmap_print(BitMap *m);
#endif