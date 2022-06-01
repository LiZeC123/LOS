#include "../kernel/bitmap.h"
#include "test.h"

int main() {
  uint8_t buf[5];
  BitMap m;
  m.bits = buf;
  m.btmp_bytes_len = sizeof(buf);

  bitmap_init(&m);

  ASSERT(bitmap_scan_test(&m, 0) == 0);
  ASSERT(bitmap_scan_test(&m, 32) == 0);
  ASSERT(bitmap_scan_test(&m, 35) == 0);
  ASSERT(bitmap_scan_test(&m, 37) == 0);

  bitmap_set(&m, 1, 1);
  bitmap_set(&m, 7, 1);
  bitmap_set(&m, 32, 1);
  bitmap_set(&m, 0, 0);

  ASSERT(bitmap_scan_test(&m, 0) == false);
  ASSERT(bitmap_scan_test(&m, 32) == true);
  ASSERT(bitmap_scan_test(&m, 7) == true);
  ASSERT(bitmap_scan_test(&m, 1) == true);

  bitmap_set(&m, 0, 1);

  ASSERT(bitmap_scan(&m, 1) == 2);
  bitmap_set(&m, 2, 1);

  ASSERT(bitmap_scan(&m, 1) == 3);
  bitmap_set(&m, 3, 1);
  bitmap_set(&m, 4, 1);

  ASSERT(bitmap_scan(&m, 7) == 8);
  bitmap_set(&m, 15, 1);

  ASSERT(bitmap_scan(&m, 30) == -1);

  return 0;
}