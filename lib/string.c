#include "string.h"
#include "debug.h"

// 从dst开始的size字节设置为value
void memset(void *dst, uint8_t value, uint32_t size) {
  ASSERT(dst != NULL);

  uint8_t *pd = (uint8_t *)dst;
  while (size-- > 0) {
    *pd++ = value;
  }
}

// 将src处的size个字节复制到dst处
void memcpy(void *dst, const void *src, uint32_t size) {
  ASSERT(dst != NULL && src != NULL);

  uint8_t *pd = (uint8_t *)dst;
  const uint8_t *ps = (const uint8_t *)src;

  while (size-- > 0) {
    *pd++ = *ps++;
  }
}

// 比较a和b处的size个字节, 大于返回1, 小于返回-1, 相等返回0
int8_t memcmp(const void *a, const void *b, uint32_t size) {
  ASSERT(a != NULL && b != NULL);

  const char *pa = (const char *)a;
  const char *pb = (const char *)b;
  while (size-- > 0) {
    if (*pa != *pb) {
      return *pa > *pb ? 1 : -1;
    }
    pa++;
    pb++;
  }

  return 0;
}

// 将字符串src中的内容复制到dst开始的位置
char *strcpy(char *dst, const char *src) {
  ASSERT(dst != NULL && src != NULL);

  char *r = dst;
  while ((*dst++ = *src++)) {
  };
  return r;
}

// 返回字符串的长度
uint32_t strlen(const char *str) {
  ASSERT(str != NULL);

  const char *p = str;
  while (*p++) {
  }

  return (p - str - 1);
}

// 比较两个字符串, 大于返回1, 小于返回-1, 相等返回0
int8_t strcmp(const char *a, const char *b) {
  ASSERT(a != NULL && b != NULL);

  while (*a != 0 && *a == *b) {
    a++;
    b++;
  }

  return *a < *b ? -1 : *a > *b;
}

// 查找ch第一次出现在str的位置, 不存在则返回NULL
char *strchr(const char *str, uint8_t ch) {
  ASSERT(str != NULL);

  while (*str != 0) {
    if (*str == ch) {
      return (char *)str;
    }
    str++;
  }

  return NULL;
}

// 从后向前查找字符ch在str中首次出现的位置, 不存在则返回NULL
char *strrchr(const char *str, const uint8_t ch) {
  ASSERT(str != NULL);

  const char *last = NULL;
  while (*str) {
    if (*str == ch) {
      last = str;
    }
    str++;
  }
  return (char *)last;
}

// 将字符串src拼接在字符串dst之后
char *strcat(char *dst, const char *src) {
  ASSERT(dst != NULL && src != NULL);

  char *ps = dst;
  while (*ps) {
    ps++;
  }

  while ((*ps++ = *src++)) {
  }

  return dst;
}

// 统计字符ch在字符串str中出现的次数
uint32_t strchrs(const char *str, uint8_t ch) {
  ASSERT(str != NULL);

  uint32_t count = 0;

  while (*str) {
    if (*str == ch) {
      count++;
    }
    str++;
  }
  return count;
}