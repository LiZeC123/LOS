#pragma once

#include "bitmap.h"
#include <stdint.h>

typedef struct {
  BitMap vaddr_map;
  uint32_t vaddr_start;
} VirtualAddr;

void mem_init();