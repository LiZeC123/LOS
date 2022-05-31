#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "../kernel/string.h"


void test_memset() {
  char arr[] = {'A', 'B', 'C'};

  memset(arr, 0, sizeof(arr));

  for (unsigned int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
    ASSERT(arr[i] == 0);
  }
}

void test_memcpy() {
  int arr[10];
  int brr[] = {0x12345678, 0xcafebabe, 0x0000ffff, 0xacf41231};

  memcpy(arr, brr, sizeof(brr));

  for (unsigned int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
    ASSERT(arr[i] == brr[i]);
  }
}

// // 比较a和b处的size个字节, 大于返回1, 小于返回-1, 相等返回0
// int8_t memcmp(const void* a, const void* b, uint32_t size);

void test_memcmp() {}

// // 将字符串src中的内容复制到dst开始的位置
// char* strcpy(char* dst, const char* src);

void test_strcpy() {}

// // 返回字符串的长度
// uint32_t strlen(const char* str);
void test_strlen() {}

// // 比较两个字符串, 大于返回1, 小于返回-1, 相等返回0
// int8_t strcmp(const char* a, const char* b);
void test_strcmp() {}

// // 查找ch第一次出现在str的位置, 不存在则返回NULL
// char* strchr(const char* str, uint8_t ch);
void test_strchr() {}

// // 从后向前查找字符ch在str中首次出现的位置, 不存在则返回NULL
// char* strrchr(const char* str, const uint8_t ch);
void test_strrchr() {}

// // 将字符串src拼接在字符串dst之后
// char* strcat(char* dst, const char* src);
void test_strcat() {}

// // 统计字符ch在字符串str中出现的次数
// uint32_t strchrs(const char* str, uint8_t ch);
void test_strchrs() {}

int main() {
  test_memset();
  test_memcmp();
  test_memcmp();
  test_strcpy();
  test_strlen();
  test_strcmp();
  test_strchr();
  test_strrchr();
  test_strcat();
  test_strchrs();
}